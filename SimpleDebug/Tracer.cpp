#include <proto/exec.h>

#include "Tracer.hpp"
#include "LowLevel.hpp"

#include <iostream>

extern struct DebugIFace *IDebug;

Tracer::Tracer(Process *process, ExceptionContext *context) {
    this->process = process;
    this->context = context;
}

bool Tracer::activate(bool branching) {
	isBranching = branching;
    if(hasTraceBit() && branching) {
        setTraceBit();
		return true;
    } else {
        if (!breaks.insert(context->ip + 4)) return false;
        uint32_t baddr = branch();
        if(baddr && branching)
            breaks.insert(baddr);
        breaks.activate();
		return true;
    }
}

void Tracer::suspend() {
    if(hasTraceBit() && isBranching) {
        unsetTraceBit();
    } else {
        breaks.deactivate();
        breaks.clear();
    }
}

uint32_t Tracer::branch()
{
	if(!is_readable_address(context->ip)) {
		return 0x0;
	}
	int32 offset;
	switch(PPC_DisassembleBranchInstr(*(uint32 *)context->ip, &offset))
	{
		case PPC_OTHER:
			return 0x0;
		case PPC_BRANCHTOLINK:
		case PPC_BRANCHTOLINKCOND:
			return context->lr;
		case PPC_BRANCHTOCTR:
		case PPC_BRANCHTOCTRCOND:
			return context->ctr;
		case PPC_BRANCH:
		case PPC_BRANCHCOND:
			return context->ip + offset;
	}
	return 0x0;
}

bool Tracer::isBranchToLink(uint32_t address)
{
	int32 dummy;
	// simply continue, if we have reached the end of the function
	if(is_readable_address(address) && PPC_DisassembleBranchInstr(*(uint32 *)address, &dummy) == PPC_BRANCHTOLINK) {
		return true;
	}
	return false;

}

#define    MSR_TRACE_ENABLE           0x00000400

void Tracer::setTraceBit ()
{
	struct ExceptionContext ctx;

 	IExec->CacheClearE((APTR)&ctx, 0xffffffff, CACRF_ClearI| CACRF_ClearD);
	IDebug->ReadTaskContext((struct Task *)process, &ctx, RTCF_STATE);
 	IExec->CacheClearE((APTR)&ctx, 0xffffffff, CACRF_ClearI| CACRF_ClearD);

	//this is not supported on the sam cpu:
	ctx.msr |= MSR_TRACE_ENABLE;

 	IExec->CacheClearE((APTR)&ctx, 0xffffffff, CACRF_ClearI| CACRF_ClearD);
	IDebug->WriteTaskContext((struct Task *)process, &ctx, RTCF_STATE);
	IExec->CacheClearE((APTR)&ctx, 0xffffffff, CACRF_ClearI| CACRF_ClearD);
}

void Tracer::unsetTraceBit ()
{
	struct ExceptionContext ctx;

 	IExec->CacheClearE((APTR)&ctx, 0xffffffff, CACRF_ClearI| CACRF_ClearD);
	IDebug->ReadTaskContext ((struct Task *)process, &ctx, RTCF_STATE);
	IExec->CacheClearE((APTR)&ctx, 0xffffffff, CACRF_ClearI| CACRF_ClearD);

	//this is not supported on the sam cpu:
	ctx.msr &= ~MSR_TRACE_ENABLE;

 	IExec->CacheClearE((APTR)&ctx, 0xffffffff, CACRF_ClearI| CACRF_ClearD);
	IDebug->WriteTaskContext((struct Task *)process, &ctx, RTCF_STATE);
	IExec->CacheClearE((APTR)&ctx, 0xffffffff, CACRF_ClearI| CACRF_ClearD);
}

bool Tracer::hasTraceBit()
{
	uint32 family;
	IExec->GetCPUInfoTags(GCIT_Family, &family, TAG_DONE);
	// if (family == CPUFAMILY_4XX || family == CPUFAMILY_E5500)
		return false;
	return true;
}