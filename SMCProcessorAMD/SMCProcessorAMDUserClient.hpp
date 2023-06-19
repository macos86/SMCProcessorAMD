//
//  SMCProcessorAMDUserClient.hpp
//  SMCProcessorAMD
//
//

#ifndef SMCProcessorAMDUserClient_hpp
#define SMCProcessorAMDUserClient_hpp


#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IOLib.h>

#include "SMCProcessorAMD.hpp"

class SMCProcessorAMDUserClient : public IOUserClient
{

    OSDeclareDefaultStructors(SMCProcessorAMDUserClient)
    
      
public:
    // IOUserClient methods
    virtual void stop(IOService* provider) override;
    virtual bool start(IOService* provider) override;
    

    
protected:
    
    SMCProcessorAMD *fProvider;
    
    // KPI for supporting access from both 32-bit and 64-bit user processes beginning with Mac OS X 10.5.
    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments* arguments,
                                    IOExternalMethodDispatch* dispatch, OSObject* target, void* reference) override;

};

#endif
