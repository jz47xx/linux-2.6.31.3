/*
 *  Kernel Probes (KProbes)
 *  arch/mips/kernel/kprobes.c
 *
 *  Copyright 2006 Sony Corp.
 *  Copyright 2010 Cavium Networks
 *
 *  Some portions copied from the powerpc version.
 *
 *   Copyright (C) IBM Corporation, 2002, 2004
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kprobes.h>
#include <linux/preempt.h>
#include <linux/kdebug.h>
#include <linux/slab.h>
#include <linux/module.h>

#include <asm/r4kcache.h>
#include <asm/ptrace.h>
#include <asm/break.h>
#include <asm/inst.h>
#include <asm/sstep.h>

static const union mips_instruction breakpoint_insn = {
	.b_format = {
		.opcode = spec_op,
		.code = BRK_KPROBE_BP,
		.func = break_op
	}
};

static const union mips_instruction breakpoint2_insn = {
	.b_format = {
		.opcode = spec_op,
		.code = BRK_KPROBE_SSTEPBP,
		.func = break_op
	}
};

DEFINE_PER_CPU(struct kprobe *, current_kprobe);
DEFINE_PER_CPU(struct kprobe_ctlblk, kprobe_ctlblk);

struct kretprobe_blackpoint kretprobe_blacklist[] = {
	{"add_partial", }, /* unknown reason, preempt error */

	{NULL, NULL}	/* Terminator */
};
const int kretprobe_blacklist_size = ARRAY_SIZE(kretprobe_blacklist);

void __kprobes _blast_cache_range(unsigned long start, unsigned long end)
{
	unsigned long lsize = cpu_dcache_line_size();
	unsigned long addr = start & ~(lsize - 1);
	unsigned long aend = (end - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize) {
		protected_cache_op(Hit_Writeback_Inv_D, addr);
		protected_cache_op(Hit_Invalidate_I, addr);
	}
	SYNC_WB();
}

#define IS_BREAK(instr)		(((instr) & 0xfc00003f) == 0x0000000d)
int __kprobes arch_prepare_kprobe(struct kprobe *p)
{
	int ret = 0;
	kprobe_opcode_t insn = *p->addr;
	int insn_step = test_step(insn.word);

	if ((unsigned long)p->addr & 0x03) {
		pr_notice("Cann't register kprobe at an unaligned address\n");
		ret = -EINVAL;
	} else if (IS_BREAK(insn.word) || IS_ERET(insn.word)) {
		pr_notice("Cann't register a kprobe on break or eret\n");
		ret = -EINVAL;
	} else if (insn_step <0) {
		printk("Register a kprobe on instruction cann't emulate."
			"%08x@%p\n",insn.word,p->addr);
		ret = -EINVAL;
	} else 	if(insn_step >0) {
	        //printk("Register a kprobe on jmp/bra %08x@%p\n",insn,p->addr);
		//ret = -EINVAL;
	} else 	{
		kprobe_opcode_t insnp = *(p->addr - 1);
		//if(insn_is_likely(insn)) {
		if(0) {
			pr_notice("Cannot register a kprobe on likely\n");
			ret = -EINVAL;
		}
		if(insn_is_jmpbra(insnp.word)) {
			pr_notice("Cannot register a kprobe in dalyslot\n");
			ret = -EINVAL;
		}
	}


	/* insn must be on a special executable page on mips. */
	if (!ret) {
		p->ainsn.insn = get_insn_slot();
		if (!p->ainsn.insn) ret = -ENOMEM;
	}

	if (!ret) {
		memcpy(p->ainsn.insn, p->addr,
		       2 * sizeof(kprobe_opcode_t));
		/*
		 * one instruction followed with break SSTEP,
		 * so to simulate single step
		 */
		if(insn_step == 0) {
			p->ainsn.insn[1] = breakpoint2_insn;
		} else {
			/* WARNING("kprobe on jmp/bra\n"); */
			p->ainsn.insn[2] = breakpoint2_insn;
		}
		p->opcode = *p->addr;
		_blast_cache_range((unsigned long)p->ainsn.insn,
			(unsigned long)p->ainsn.insn + 3*sizeof(kprobe_opcode_t));
	}

	return ret;
}

void __kprobes arch_arm_kprobe(struct kprobe *p)
{
	*p->addr = breakpoint_insn;
	flush_insn_slot(p);
}

void __kprobes arch_disarm_kprobe(struct kprobe *p)
{
	*p->addr = p->opcode;
	flush_insn_slot(p);
}

void __kprobes arch_remove_kprobe(struct kprobe *p)
{
	free_insn_slot(p->ainsn.insn, 0);
}

static void save_previous_kprobe(struct kprobe_ctlblk *kcb)
{
	kcb->prev_kprobe.kp = kprobe_running();
	kcb->prev_kprobe.status = kcb->kprobe_status;
	kcb->prev_kprobe.old_SR = kcb->kprobe_old_SR;
	kcb->prev_kprobe.saved_SR = kcb->kprobe_saved_SR;
	kcb->prev_kprobe.saved_epc = kcb->kprobe_saved_epc;
}

static void restore_previous_kprobe(struct kprobe_ctlblk *kcb)
{
	__get_cpu_var(current_kprobe) = kcb->prev_kprobe.kp;
	kcb->kprobe_status = kcb->prev_kprobe.status;
	kcb->kprobe_old_SR = kcb->prev_kprobe.old_SR;
	kcb->kprobe_saved_SR = kcb->prev_kprobe.saved_SR;
	kcb->kprobe_saved_epc = kcb->prev_kprobe.saved_epc;
}

static void set_current_kprobe(struct kprobe *p, struct pt_regs *regs,
			       struct kprobe_ctlblk *kcb)
{
	__get_cpu_var(current_kprobe) = p;
	kcb->kprobe_saved_SR = kcb->kprobe_old_SR = (regs->cp0_status & ST0_IE);
	kcb->kprobe_saved_epc = regs->cp0_epc;
}

static void prepare_singlestep(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();
	kprobe_opcode_t insn = p->ainsn.insn[0];
	int ret = emulate_step(regs, insn.word);

	if(ret != 0) {
		if(ret == 1) /* bra/jmp instruction jumped */
			kcb->kprobe_saved_epc = regs->cp0_epc;
		else
			kcb->kprobe_saved_epc += 4;
		regs->cp0_epc = (unsigned long)(&p->ainsn.insn[1]);
	} else {
		regs->cp0_epc = (unsigned long)(&p->ainsn.insn[0]);
	}
	regs->cp0_status &= ~ST0_IE;
}

static int __kprobes kprobe_handler(struct pt_regs *regs)
{
	struct kprobe *p;
	int ret = 0;
	kprobe_opcode_t *addr;
	struct kprobe_ctlblk *kcb;

	addr = (kprobe_opcode_t *) regs->cp0_epc;

	/*
	 * We don't want to be preempted for the entire
	 * duration of kprobe processing
	 */
	preempt_disable();
	kcb = get_kprobe_ctlblk();

	/* Check we're not actually recursing */
	if (kprobe_running()) {
		p = get_kprobe(addr);
		if (p) {
			if (kcb->kprobe_status == KPROBE_HIT_SS &&
			    p->ainsn.insn->word == breakpoint_insn.word) {
				regs->cp0_status &= ~ST0_IE;
				regs->cp0_status |= kcb->kprobe_saved_SR;
				goto no_kprobe;
			}
			/*
			 * We have reentered the kprobe_handler(), since
			 * another probe was hit while within the handler.
			 * We here save the original kprobes variables and
			 * just single step on the instruction of the new probe
			 * without calling any user handlers.
			 */
			save_previous_kprobe(kcb);
			set_current_kprobe(p, regs, kcb);
			kprobes_inc_nmissed_count(p);
			prepare_singlestep(p, regs);
			kcb->kprobe_status = KPROBE_REENTER;
			return 1;
		} else {
			if (addr->word != breakpoint_insn.word) {
				/*
				 * The breakpoint instruction was removed by
				 * another cpu right after we hit, no further
				 * handling of this interrupt is appropriate
				 */
				ret = 1;
				goto no_kprobe;
			}
			p = __get_cpu_var(current_kprobe);
			if (p->break_handler && p->break_handler(p, regs))
				goto ss_probe;
		}
		goto no_kprobe;
	}

	p = get_kprobe(addr);
	if (!p) {
		if (addr->word != breakpoint_insn.word) {
			/*
			 * The breakpoint instruction was removed right
			 * after we hit it.  Another cpu has removed
			 * either a probepoint or a debugger breakpoint
			 * at this address.  In either case, no further
			 * handling of this interrupt is appropriate.
			 */
			ret = 1;
		}
		/* Not one of ours: let kernel handle it */
		goto no_kprobe;
	}

	set_current_kprobe(p, regs, kcb);
	kcb->kprobe_status = KPROBE_HIT_ACTIVE;

	if (p->pre_handler && p->pre_handler(p, regs)) {
		/* handler has already set things up, so skip ss setup */
		return 1;
	}

ss_probe:
	prepare_singlestep(p, regs);
	kcb->kprobe_status = KPROBE_HIT_SS;
	return 1;

no_kprobe:
	preempt_enable_no_resched();
	return ret;

}

/*
 * Called after single-stepping.  p->addr is the address of the
 * instruction whose first byte has been replaced by the "break 0"
 * instruction.  To avoid the SMP problems that can occur when we
 * temporarily put back the original opcode to single-step, we
 * single-stepped a copy of the instruction.  The address of this
 * copy is p->ainsn.insn.
 *
 * This function prepares to return from the post-single-step
 * breakpoint trap.
 */
static void __kprobes resume_execution(struct kprobe *p,
				       struct pt_regs *regs,
				       struct kprobe_ctlblk *kcb)
{
	unsigned long orig_epc = kcb->kprobe_saved_epc;
	int is_jmp = p->ainsn.insn[1].word != breakpoint2_insn.word;
	if(is_jmp)
		regs->cp0_epc = orig_epc;
	else
		regs->cp0_epc = orig_epc + 4;
}

static inline int post_kprobe_handler(struct pt_regs *regs)
{
	struct kprobe *cur = kprobe_running();
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();

	if (!cur)
		return 0;

	if ((kcb->kprobe_status != KPROBE_REENTER) && cur->post_handler) {
		kcb->kprobe_status = KPROBE_HIT_SSDONE;
		cur->post_handler(cur, regs, 0);
	}

	resume_execution(cur, regs, kcb);

	regs->cp0_status |= kcb->kprobe_saved_SR;

	/* Restore back the original saved kprobes variables and continue. */
	if (kcb->kprobe_status == KPROBE_REENTER) {
		restore_previous_kprobe(kcb);
		goto out;
	}
	reset_current_kprobe();
out:
	preempt_enable_no_resched();

	return 1;
}

static inline int kprobe_fault_handler(struct pt_regs *regs, int trapnr)
{
	struct kprobe *cur = kprobe_running();
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();

	if (cur->fault_handler && cur->fault_handler(cur, regs, trapnr))
		return 1;

	if (kcb->kprobe_status & KPROBE_HIT_SS) {
		resume_execution(cur, regs, kcb);
		regs->cp0_status |= kcb->kprobe_old_SR;

		reset_current_kprobe();
		preempt_enable_no_resched();
	}
	return 0;
}

/*
 * Wrapper routine for handling exceptions.
 */
int __kprobes kprobe_exceptions_notify(struct notifier_block *self,
				       unsigned long val, void *data)
{

	struct die_args *args = (struct die_args *)data;
	int ret = NOTIFY_DONE;

	switch (val) {
	case DIE_BREAK:
		if (kprobe_handler(args->regs))
			ret = NOTIFY_STOP;
		break;
	case DIE_SSTEPBP:
		if (post_kprobe_handler(args->regs))
			ret = NOTIFY_STOP;
		break;

	case DIE_PAGE_FAULT:
		/* kprobe_running() needs smp_processor_id() */
		preempt_disable();

		if (kprobe_running()
		    && kprobe_fault_handler(args->regs, args->trapnr))
			ret = NOTIFY_STOP;
		preempt_enable();
		break;
	default:
		break;
	}
	return ret;
}

int __kprobes setjmp_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct jprobe *jp = container_of(p, struct jprobe, kp);
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();

	kcb->jprobe_saved_regs = *regs;
	kcb->jprobe_saved_sp = regs->regs[29];

	memcpy(kcb->jprobes_stack, (void *)kcb->jprobe_saved_sp,
	       MIN_JPROBES_STACK_SIZE(kcb->jprobe_saved_sp));

	regs->cp0_epc = (unsigned long)(jp->entry);

	return 1;
}

/* Defined in the inline asm below. */
void jprobe_return_end(void);

void __kprobes jprobe_return(void)
{
	/* Assembler quirk necessitates this '0,code' business.  */
	asm volatile(
		"break 0,%0\n\t"
		".globl jprobe_return_end\n"
		"jprobe_return_end:\n"
		: : "n" (BRK_KPROBE_BP) : "memory");
}

int __kprobes longjmp_break_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();

	if (regs->cp0_epc >= (unsigned long)jprobe_return &&
	    regs->cp0_epc <= (unsigned long)jprobe_return_end) {
		*regs = kcb->jprobe_saved_regs;
		memcpy((void *)kcb->jprobe_saved_sp, kcb->jprobes_stack,
		       MIN_JPROBES_STACK_SIZE(kcb->jprobe_saved_sp));
		preempt_enable_no_resched();

		return 1;
	}
	return 0;
}

/*
 * Function return probe trampoline:
 *	- init_kprobes() establishes a probepoint here
 *	- When the probed function returns, this probe causes the
 *	  handlers to fire
 */
static void __used kretprobe_trampoline_holder(void)
{
	asm volatile(
		".set push\n\t"
		/* Keep the assembler from reordering and placing JR here. */
		".set noreorder\n\t"
		"nop\n\t"
		".global kretprobe_trampoline\n"
		"kretprobe_trampoline:\n\t"
		"nop\n\t"
		".set pop"
		: : : "memory");
}

void kretprobe_trampoline(void);

void __kprobes arch_prepare_kretprobe(struct kretprobe_instance *ri,
				      struct pt_regs *regs)
{
	ri->ret_addr = (kprobe_opcode_t *) regs->regs[31];

	/* Replace the return addr with trampoline addr */
	regs->regs[31] = (unsigned long)kretprobe_trampoline;
}

/*
 * Called when the probe at kretprobe trampoline is hit
 */
static int __kprobes trampoline_probe_handler(struct kprobe *p,
						struct pt_regs *regs)
{
	struct kretprobe_instance *ri = NULL;
	struct hlist_head *head, empty_rp;
	struct hlist_node *node, *tmp;
	unsigned long flags, orig_ret_address = 0;
	unsigned long trampoline_address = (unsigned long)kretprobe_trampoline;

	INIT_HLIST_HEAD(&empty_rp);
	kretprobe_hash_lock(current, &head, &flags);

	/*
	 * It is possible to have multiple instances associated with a given
	 * task either because an multiple functions in the call path
	 * have a return probe installed on them, and/or more than one return
	 * return probe was registered for a target function.
	 *
	 * We can handle this because:
	 *     - instances are always inserted at the head of the list
	 *     - when multiple return probes are registered for the same
	 *       function, the first instance's ret_addr will point to the
	 *       real return address, and all the rest will point to
	 *       kretprobe_trampoline
	 */
	hlist_for_each_entry_safe(ri, node, tmp, head, hlist) {
		if (ri->task != current)
			/* another task is sharing our hash bucket */
			continue;

		if (ri->rp && ri->rp->handler)
			ri->rp->handler(ri, regs);

		orig_ret_address = (unsigned long)ri->ret_addr;
		recycle_rp_inst(ri, &empty_rp);

		if (orig_ret_address != trampoline_address)
			/*
			 * This is the real return address. Any other
			 * instances associated with this task are for
			 * other calls deeper on the call stack
			 */
			break;
	}

	kretprobe_assert(ri, orig_ret_address, trampoline_address);
	instruction_pointer(regs) = orig_ret_address;

	reset_current_kprobe();
	kretprobe_hash_unlock(current, &flags);
	preempt_enable_no_resched();

	hlist_for_each_entry_safe(ri, node, tmp, &empty_rp, hlist) {
		hlist_del(&ri->hlist);
		kfree(ri);
	}
	/*
	 * By returning a non-zero value, we are telling
	 * kprobe_handler() that we don't want the post_handler
	 * to run (and have re-enabled preemption)
	 */
	return 1;
}

int __kprobes arch_trampoline_kprobe(struct kprobe *p)
{
	if (p->addr == (kprobe_opcode_t *)kretprobe_trampoline)
		return 1;

	return 0;
}

static struct kprobe trampoline_p = {
	.addr = (kprobe_opcode_t *)kretprobe_trampoline,
	.pre_handler = trampoline_probe_handler
};

int __init arch_init_kprobes(void)
{
	return register_kprobe(&trampoline_p);
}



/*
 * Used for Systemtap mips support. stack-mips.c
 */
#include <asm/stacktrace.h>
unsigned long _mips_unwind_stack(struct task_struct *task, unsigned long *sp,
			   unsigned long pc, unsigned long *ra)
{
	if(!task) task = current;
	if(!task) BUG();
	return unwind_stack(task,sp,pc,ra);
}
EXPORT_SYMBOL(_mips_unwind_stack);
