#include "SMCProcessorAMD.hpp"


OSDefineMetaClassAndStructors(SMCProcessorAMD, IOService);

#define TCTL_OFFSET_TABLE_LEN 6
static constexpr const struct tctl_offset tctl_offset_table[] = {
    { 0x17, "AMD Ryzen 5 1600X", 20 },
    { 0x17, "AMD Ryzen 7 1700X", 20 },
    { 0x17, "AMD Ryzen 7 1800X", 20 },
    { 0x17, "AMD Ryzen 7 2700X", 10 },
    { 0x17, "AMD Ryzen Threadripper 19", 27 }, /* 19{00,20,50}X */
    { 0x17, "AMD Ryzen Threadripper 29", 27 }, /* 29{20,50,70,90}[W]X */
};


bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

bool SMCProcessorAMD::init(OSDictionary *dictionary){
    
    IOLog("SMCProcessorAMD v%s, init\n", xStringify(MODULE_VERSION));
    
    return IOService::init(dictionary);
}

void SMCProcessorAMD::free(){
    IOService::free();
}

bool SMCProcessorAMD::setupKeysVsmc(){
    
    vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    
    bool suc = true;
   
    //Read watt cpu
    suc &= VirtualSMCAPI::addKey(KeyPCPR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyPCPT, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyPCTR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(this, 0)));
    
    // Cpu TEMP
    //suc &= VirtualSMCAPI::addKey(KeyTCxD(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxE(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxF(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxG(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78));
    //suc &= VirtualSMCAPI::addKey(KeyTCxH(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxJ(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78));
    suc &= VirtualSMCAPI::addKey(KeyTCxP(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxT(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxp(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
     
        if(!suc){
        IOLog("SMCProcessorAMD::setupKeysVsmc: VirtualSMCAPI::addKey returned false. \n");
    }
    
    return suc;
}

bool SMCProcessorAMD::vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier) {
    if (sensors && vsmc) {
        IOLog("SMCProcessorAMD: got vsmc notification\n");
        auto &plugin = static_cast<SMCProcessorAMD *>(sensors)->vsmcPlugin;
        auto ret = vsmc->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, sensors, &plugin, nullptr, nullptr);
        if (ret == kIOReturnSuccess) {
            IOLog("SMCProcessorAMD: submitted plugin\n");
            return true;
        } else if (ret != kIOReturnUnsupported) {
            IOLog("SMCProcessorAMD: plugin submission failure %X\n", ret);
        } else {
            IOLog("SMCProcessorAMD: plugin submission to non vsmc\n");
        }
    } else {
        IOLog("SMCProcessorAMD: got null vsmc notification\n");
    }
    return false;
}


bool SMCProcessorAMD::getPCIService(){
    

    OSDictionary *matching_dict = serviceMatching("IOPCIDevice");
    if(!matching_dict){
        IOLog("SMCProcessorAMD::getPCIService: serviceMatching unable to generate matching dictonary.\n");
        return false;
    }
    
    //Wait for PCI services to init.
    waitForMatchingService(matching_dict);
    
    OSIterator *service_iter = getMatchingServices(matching_dict);
    IOPCIDevice *service = 0;
    
    if(!service_iter){
        IOLog("SMCProcessorAMD::getPCIService: unable to find a matching IOPCIDevice.\n");
        return false;
    }
 
    while (true){
        OSObject *obj = service_iter->getNextObject();
        if(!obj) break;
        
        service = OSDynamicCast(IOPCIDevice, obj);
        break;
    }
    
    if(!service){
        IOLog("SMCProcessorAMD::getPCIService: unable to get IOPCIDevice on host system.\n");
        return false;
    }
    
    IOLog("SMCProcessorAMD::getPCIService: succeed!\n");
    fIOPCIDevice = service;
    
    return true;
}


bool SMCProcessorAMD::start(IOService *provider){
    
    bool success = IOService::start(provider);
    if(!success){
        IOLog("SMCProcessorAMD::start failed to start. :(\n");
        return false;
    }
    registerService();
    
    //cpuGeneration = CPUInfo::getGeneration(&cpuFamily, &cpuModel, &cpuStepping);
    
    uint32_t cpuid_eax = 0;
    uint32_t cpuid_ebx = 0;
    uint32_t cpuid_ecx = 0;
    uint32_t cpuid_edx = 0;
    CPUInfo::getCpuid(0, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    IOLog("SMCProcessorAMD::start got CPUID: %X %X %X %X\n", cpuid_eax, cpuid_ebx, cpuid_ecx, cpuid_edx);
    
    if(cpuid_ebx != CPUInfo::signature_AMD_ebx
       || cpuid_ecx != CPUInfo::signature_AMD_ecx
       || cpuid_edx != CPUInfo::signature_AMD_edx){
        IOLog("SMCProcessorAMD::start no AMD signature detected, failing..\n");
        
        return false;
    }
    
    CPUInfo::getCpuid(1, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    cpuFamily = ((cpuid_eax >> 20) & 0xff) + ((cpuid_eax >> 8) & 0xf);
    cpuModel = ((cpuid_eax >> 16) & 0xf) + ((cpuid_eax >> 4) & 0xf);
    
    //Only 17h Family are supported offically by now.
    cpuSupportedByCurrentVersion = (cpuFamily == 0x17)? 1 : 0;
    IOLog("SMCProcessorAMD::start Family %02Xh, Model %02Xh\n", cpuFamily, cpuModel);
    
    CPUInfo::getCpuid(0x80000005, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    cpuCacheL1_perCore = (cpuid_ecx >> 24) + (cpuid_ecx >> 24);
    
    
    CPUInfo::getCpuid(0x80000006, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    cpuCacheL2_perCore = (cpuid_ecx >> 16);
    cpuCacheL3 = (cpuid_edx >> 18) * 512;
    IOLog("SMCProcessorAMD::start L1: %u, L2: %u, L3: %u\n",
          cpuCacheL1_perCore, cpuCacheL2_perCore, cpuCacheL3);
    
    
    CPUInfo::getCpuid(0x80000007, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    cpbSupported = (cpuid_edx >> 9) & 0x1;
    
    uint32_t nameString[12]{};
    CPUInfo::getCpuid(0x80000002, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    nameString[0] = cpuid_eax; nameString[1] = cpuid_ebx; nameString[2] = cpuid_ecx; nameString[3] = cpuid_edx;
    CPUInfo::getCpuid(0x80000003, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    nameString[4] = cpuid_eax; nameString[5] = cpuid_ebx; nameString[6] = cpuid_ecx; nameString[7] = cpuid_edx;
    CPUInfo::getCpuid(0x80000004, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    nameString[8] = cpuid_eax; nameString[9] = cpuid_ebx; nameString[10] = cpuid_ecx; nameString[11] = cpuid_edx;
    
    IOLog("SMCProcessorAMD::start Processor: %s))\n", (char*)nameString);
    
    //Check tctl temperature offset
    for(int i = 0; i < TCTL_OFFSET_TABLE_LEN; i++){
        const TempOffset *to = tctl_offset_table + i;
        IOLog("############%s##########\n", to->id);
        if(cpuFamily == to->model && strstr((char*)nameString, to->id)){
            
            tempOffset = (float)to->offset;
            break;
        }
    }
    
    
    if(!CPUInfo::getCpuTopology(cpuTopology)){
        IOLog("SMCProcessorAMD::start unable to get CPU Topology.\n");
    }
    IOLog("SMCProcessorAMD::start got %hhu CPU(s): Physical Count: %hhu, Logical Count %hhu.\n",
          cpuTopology.packageCount, cpuTopology.totalPhysical(), cpuTopology.totalLogical());
    
    totalNumberOfPhysicalCores = cpuTopology.totalPhysical();
    totalNumberOfLogicalCores = cpuTopology.totalLogical();
    
    
    workLoop = IOWorkLoop::workLoop();
    timerEventSource = IOTimerEventSource::timerEventSource(this, [](OSObject *object, IOTimerEventSource *sender) {
        SMCProcessorAMD *provider = OSDynamicCast(SMCProcessorAMD, object);
        
        
        mp_rendezvous_no_intrs([](void *obj) {
            auto provider = static_cast<SMCProcessorAMD*>(obj);
            
            //Read current clock speed from MSR for each core
            provider->updateClockSpeed();
        }, provider);
        
        //Read stats from package.
        provider->updatePackageTemp();
        provider->updatePackageEnergy();
        
        provider->timerEventSource->setTimeoutMS(1000);
    });
        
    IOLog("SMCProcessorAMD::start trying to init PCI service...\n");
    if(!getPCIService()){
        IOLog("SMCProcessorAMD::start no PCI support found, failing...\n");
        return false;
    }
    
    
    lastUpdateTime = getCurrentTimeNs();
    
    workLoop->addEventSource(timerEventSource);
    timerEventSource->setTimeoutMS(1000);
    
    IOLog("SMCProcessorAMD::start registering VirtualSMC keys...\n");
    setupKeysVsmc();
    
    return success;
}

void SMCProcessorAMD::stop(IOService *provider){
    IOLog("SMCProcessorAMD stopped, you have no more support :(\n");
    
    timerEventSource->cancelTimeout();
    
    IOService::stop(provider);
}

bool SMCProcessorAMD::read_msr(uint32_t addr, uint64_t *value){
    
    uint32_t lo, hi;
//    IOLog("SMCProcessorAMD lalala \n");
    int err = rdmsr_carefully(addr, &lo, &hi);
//    IOLog("SMCProcessorAMD rdmsr_carefully %d\n", err);
    
    if(!err) *value = lo | ((uint64_t)hi << 32);
    
    return err == 0;
}

void SMCProcessorAMD::updateClockSpeed(){
    
    uint32_t cpu_num = cpu_number();
            
    // Ignore hyper-threaded cores
    uint8_t package = cpuTopology.numberToPackage[cpu_num];
    uint8_t logical = cpuTopology.numberToLogical[cpu_num];
    if (logical >= cpuTopology.physicalCount[package])
        return;
            
    uint8_t physical = cpuTopology.numberToPhysicalUnique(cpu_num);
            
    uint64_t msr_value_buf = 0;
    bool err = !read_msr(kMSR_HARDWARE_PSTATE_STATUS, &msr_value_buf);
    if(err) IOLog("SMCProcessorAMD::updateClockSpeed: failed somewhere");
            
//    IOLog("SMCProcessorAMD::updateClockSpeed: i am CPU %hhu, physical %hhu\n", package, physical);
            
    MSR_HARDWARE_PSTATE_STATUS_perCore[physical] = msr_value_buf;
}

void SMCProcessorAMD::updatePackageTemp(){
    
    IOPCIAddressSpace space;
    space.bits = 0x00;
    
    fIOPCIDevice->configWrite32(space, (UInt8)kFAMILY_17H_PCI_CONTROL_REGISTER, (UInt32)kF17H_M01H_THM_TCON_CUR_TMP);
    uint32_t temperature = fIOPCIDevice->configRead32(space, kFAMILY_17H_PCI_CONTROL_REGISTER + 4);
    
    
    bool tempOffsetFlag = (temperature & kF17H_TEMP_OFFSET_FLAG) != 0;
    temperature = (temperature >> 21) * 125;
    
    float t = temperature * 0.001f;
    
    t -= tempOffset;
    
    if (tempOffsetFlag)
        t -= 49.0f;
    
    
    PACKAGE_TEMPERATURE_perPackage[0] = t;
//    IOLog("SMCProcessorAMD::updatePackageTemp: read from pci device %d \n", (int)PACKAGE_TEMPERATURE_perPackage[0]);
}

void SMCProcessorAMD::updatePackageEnergy(){
    
    uint64_t time = getCurrentTimeNs();
    
    uint64_t msr_value_buf = 0;
    read_msr(kMSR_PKG_ENERGY_STAT, &msr_value_buf);
    
    uint32_t energyValue = (uint32_t)(msr_value_buf & 0xffffffff);
    
    uint64_t energyDelta = (lastUpdateEnergyValue <= energyValue) ?
        energyValue - lastUpdateEnergyValue : UINT64_MAX - lastUpdateEnergyValue;
    
    double e = (0.0000153 * energyDelta) / ((time - lastUpdateTime) / 1000000000.0);
    uniPackageEnergy = e;
    
    lastUpdateEnergyValue = energyValue;
    lastUpdateTime = time;
    
//    IOLog("SMCProcessorAMD::updatePackageEnergy: %d \n", (int)e);
//    IOLog("SMCProcessorAMD::updatePackageEnergy: %d la\n", (int)energyDelta);
    
}

EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
    // Report success but actually do not start and let I/O Kit unload us.
    // This works better and increases boot speed in some cases.
    PE_parse_boot_argn("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) = checkKernelArgument("-amdcpudbg");
    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) {
    // It is not safe to unload VirtualSMC plugins!
    return KERN_FAILURE;
}


#ifdef __MAC_10_15

// macOS 10.15 adds Dispatch function to all OSObject instances and basically
// every header is now incompatible with 10.14 and earlier.
// Here we add a stub to permit older macOS versions to link.
// Note, this is done in both kern_util and plugin_start as plugins will not link
// to Lilu weak exports from vtable.

kern_return_t WEAKFUNC PRIVATE OSObject::Dispatch(const IORPC rpc) {
    PANIC("util", "OSObject::Dispatch smcproc stub called");
}

kern_return_t WEAKFUNC PRIVATE OSMetaClassBase::Dispatch(const IORPC rpc) {
    PANIC("util", "OSMetaClassBase::Dispatch smcproc stub called");
}

#endif
