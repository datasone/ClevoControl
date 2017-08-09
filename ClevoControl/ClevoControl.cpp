#include <libkern/version.h>
#include <IOKit/IOCommandGate.h>
#include <sys/kern_control.h>
#include "ClevoControl.hpp"
#include "ECCtrl.h"

OSDefineMetaClassAndStructors(moe_datasone_ClevoControl, IOService)

bool ClevoControl::init(OSDictionary *dict)
{
	bool result = IOService::init(dict);
	device = nullptr;
	return result;
}

IOService *ClevoControl::probe(IOService *provider, SInt32 *score)
{
	return IOService::probe(provider, score);
}

errno_t EPHandleSet(kern_ctl_ref ctlref, unsigned int unit, void *userdata, int opt, void *data, size_t len)
{
	return 0;
}

errno_t EPHandleGet(kern_ctl_ref ctlref, unsigned int unit, void *userdata, int opt, void *data, size_t *len)
{
	return 0;
}

errno_t EPHandleConnect(kern_ctl_ref ctlref, struct sockaddr_ctl *sac, void **unitinfo)
{
	return 0;
}

errno_t EPHandleDisconnect(kern_ctl_ref ctlref, unsigned int unit, void *unitinfo)
{
	return 0;
}

errno_t ClevoControl::EPHandleWrite(kern_ctl_ref ctlref, unsigned int unit, void *userdata, mbuf_t m, int flags)
{
	OSObject *params[3];
	struct ECCtrl *ctrl;
	ctrl = (ECCtrl *)mbuf_data(m);
	params[0] = OSNumber::withNumber(ctrl->arg0, 8 * sizeof(uint32_t));
	params[1] = OSNumber::withNumber(ctrl->arg1, 8 * sizeof(uint32_t));
	params[2] = OSNumber::withNumber(ctrl->arg2, 8 * sizeof(uint32_t));
	device->evaluateObject("WMIB", nullptr, params, 3);
	return 0;
}

bool ClevoControl::start(IOService *provider)
{
	device = OSDynamicCast(IOACPIPlatformDevice, provider);
	if (device == nullptr || !IOService::start(provider))
		return false;
	errno_t error;
	struct kern_ctl_reg ep_ctl;
	kern_ctl_ref kctlref;
	bzero(&ep_ctl, sizeof(ep_ctl));
	ep_ctl.ctl_id = 0;
	ep_ctl.ctl_unit = 0;
	strcpy(ep_ctl.ctl_name, "moe.datasone.clevocontrol.ctl");
	ep_ctl.ctl_flags = CTL_FLAG_PRIVILEGED & CTL_FLAG_REG_ID_UNIT;
	ep_ctl.ctl_send = EPHandleWrite;
	ep_ctl.ctl_setopt = EPHandleSet;
	ep_ctl.ctl_getopt = EPHandleGet;
	ep_ctl.ctl_connect = EPHandleConnect;
	ep_ctl.ctl_disconnect = EPHandleDisconnect;
	error = ctl_register(&ep_ctl, &kctlref);
	return true;
}

void ClevoControl::stop(IOService *provider)
{
	
}
