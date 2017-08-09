#ifndef ClevoControl_hpp
#define ClevoControl_hpp

#define ClevoControl moe_datasone_ClevoControl

#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>

class ClevoControl : public IOService
{
	OSDeclareDefaultStructors(moe_datasone_ClevoControl)
	
public:
	virtual bool init(OSDictionary *dictionary = 0) override;
	virtual IOService *probe(IOService *provider, SInt32 *score) override;
	virtual bool start(IOService *provider) override;
	virtual void stop(IOService *provider) override;
	
	static errno_t EPHandleWrite(kern_ctl_ref ctlref, unsigned int unit, void *userdata, mbuf_t m, int flags);
};

static IOACPIPlatformDevice *device;

#endif
