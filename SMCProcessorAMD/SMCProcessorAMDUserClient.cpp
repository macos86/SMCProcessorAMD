//
//  SMCProcessorAMDUserClient.cpp
//  SMCProcessorAMD
//

#include "SMCProcessorAMDUserClient.hpp"



OSDefineMetaClassAndStructors(SMCProcessorAMDUserClient, IOUserClient);


bool SMCProcessorAMDUserClient::start(IOService *provider){
    
    IOLog("SMCProcessorAMDUserClient::start\n");
    
    bool success = IOService::start(provider);
    
    if(success){
        fProvider = OSDynamicCast(SMCProcessorAMD, provider);
    }
    
    return success;
}

void SMCProcessorAMDUserClient::stop(IOService *provider){
    IOLog("SMCProcessorAMDUserClient::stop\n");
    
    fProvider = nullptr;
    IOService::stop(provider);
}

uint64_t multiply_two_numbers(uint64_t number_one, uint64_t number_two){
    uint64_t number_three = 0;
    for(uint32_t i = 0; i < number_two; i++){
        number_three = number_three + number_one;
    }
    return number_three;
}

IOReturn SMCProcessorAMDUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
                                                 IOExternalMethodDispatch *dispatch, OSObject *target, void *reference){
    
    IOLog("SMCProcessorAMDUserClient::externalMethod selector:%d\n", selector);
    
    
    switch (selector) {
        case 0: {
            // multiply_two_numbers
            uint64_t r = multiply_two_numbers(arguments->scalarInput[0], arguments->scalarInput[1]);
            arguments->scalarOutput[0] = r;
            arguments->scalarOutputCount = 1;
            
            IOLog("SMCProcessorAMDUserClient::multiply_two_numbers r:%llu\n", r);
            
            break;
        }
        case 1: {
            // read_msr
//            IOLog("SMCProcessorAMDUserClient::read_msr: got raw address %llu\n", arguments->scalarInput[0]);
            uint32_t msr_addr = (uint32_t)(arguments->scalarInput[0]);
            uint64_t msr_value_buf = 0;
            bool err = !fProvider->read_msr(msr_addr, &msr_value_buf);
            if(err){
                IOLog("SMCProcessorAMDUserClient::read_msr: failed at address %u\n", msr_addr);
            } else {
                arguments->scalarOutput[0] = msr_value_buf;
                arguments->scalarOutputCount = 1;
            }
            
            
            break;
        }
            
        case 2: {
            
            uint32_t numPhyCores = fProvider->totalNumberOfPhysicalCores;

            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = numPhyCores;
            
            arguments->structureOutputSize = numPhyCores * sizeof(uint64_t);

            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            for(uint32_t i = 0; i < numPhyCores; i++){
                dataOut[i] = fProvider->MSR_HARDWARE_PSTATE_STATUS_perCore[i];
            }
            
            break;
        }
        
        case 3: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = 1 * sizeof(float);
            
            float *dataOut = (float*) arguments->structureOutput;
            dataOut[0] = fProvider->PACKAGE_TEMPERATURE_perPackage[0];
            break;
        }
          
            
        //Get all data like this: [power, temp, pstateCur, clock_core_1, 2, 3 .....]
        //Yes, i am too lazy to write a struct
        case 4: {
            /*uint32_t numPhyCores = fProvider->totalNumberOfPhysicalCores;
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = numPhyCores;
            
            arguments->structureOutputSize = (numPhyCores + 3) * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;
            
            dataOut[0] = (float)fProvider->uniPackageEnegry;
            dataOut[1] = fProvider->PACKAGE_TEMPERATURE_perPackage[0];
            dataOut[2] = fProvider->PStateCtl;
            
            for(uint32_t i = 0; i < numPhyCores; i++){
                dataOut[i + 3] = fProvider->effFreq_perCore[i];
            }*/
            float *dataOut = (float*) arguments->structureOutput;
            
            dataOut[0] = 40;
            dataOut[1] = 41;
            dataOut[2] = 42;
            dataOut[3] = 43;
            dataOut[4] = 44;
            dataOut[5] = 45;
            dataOut[6] = 46;
            dataOut[7] = 47;
            dataOut[8] = 48;
            dataOut[9] = 49;
            
            break;
        }
            
        //Get per core raw load index
        case 5: {
            /*arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (fProvider->totalNumberOfPhysicalCores) * sizeof(uint64_t);

            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < fProvider->totalNumberOfPhysicalCores; i++){
                dataOut[i] = fProvider->instructionDelta_PerCore[i];
            }*/
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            dataOut[0] = 50;
            dataOut[1] = 51;
            dataOut[2] = 52;
            dataOut[3] = 53;
            dataOut[4] = 54;
            dataOut[5] = 55;
            dataOut[6] = 56;
            dataOut[7] = 57;
            dataOut[8] = 58;
            dataOut[9] = 59;
            
            break;
        }
            
        //Get per core load index
        case 6: {
            /*arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (fProvider->totalNumberOfPhysicalCores) * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < fProvider->totalNumberOfPhysicalCores; i++){
                dataOut[i] = fProvider->loadIndex_PerCore[i];
            }*/
            
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            dataOut[0] = 60;
            dataOut[1] = 61;
            dataOut[2] = 62;
            dataOut[3] = 63;
            dataOut[4] = 64;
            dataOut[5] = 65;
            dataOut[6] = 66;
            dataOut[7] = 67;
            dataOut[8] = 68;
            dataOut[9] = 69;
            
            break;
        }
            
        //Get basic CPUID
        //[Family, Model, Physical, Logical, L1_perCore, L2_perCore, L3]
        case 7: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (8) * sizeof(uint64_t);

            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            dataOut[0] = (uint64_t)fProvider->cpuFamily;
            dataOut[1] = (uint64_t)fProvider->cpuModel;
            dataOut[2] = (uint64_t)fProvider->totalNumberOfPhysicalCores;
            dataOut[3] = (uint64_t)fProvider->totalNumberOfLogicalCores;
            dataOut[4] = (uint64_t)fProvider->cpuCacheL1_perCore;
            dataOut[5] = (uint64_t)fProvider->cpuCacheL2_perCore;
            dataOut[6] = (uint64_t)fProvider->cpuCacheL3;
            dataOut[7] = (uint64_t)fProvider->cpuSupportedByCurrentVersion;
            
            break;
        }
        
        //Get SMCAMDProcessor Version String
        case 8: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (uint32_t)strlen(xStringify(MODULE_VERSION));
            char *dataOut = (char*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < arguments->structureOutputSize; i++){
                dataOut[i] = xStringify(MODULE_VERSION)[i];
            }
            
            break;
        }
        
        //Get PState
        case 9: {
            /*arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = 1 * sizeof(uint64_t);
            
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = fProvider->PStateCtl;*/
            
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            dataOut[0] = 90;
            
            break;
        }
        
        //Set PState
        case 10: {
            /*arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 0;
            
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
            
            fProvider->PStateCtl = (uint8_t)arguments->scalarInput[0];
            fProvider->applyPowerControl();*/
            
            
            break;
        }
            
        //Get CPB
        case 11: {
            /*arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = 2 * sizeof(uint64_t);
            
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = (uint64_t)fProvider->cpbSupported;
            dataOut[1] = (uint64_t)fProvider->getCPBState();*/
            
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            dataOut[0] = 110;
            dataOut[1] = 111;
            break;
        }
        
        //Set CPB
        case 12: {
            /*arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 0;
            
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
            
            if(!fProvider->cpbSupported)
                return kIOReturnNoDevice;
            
            fProvider->setCPBState(arguments->scalarInput[0]==1?true:false);
            */
            break;
        }
            
        //Get PPM
        case 13: {
            /*arguments->scalarOutputCount = 0;
                
            arguments->structureOutputSize = 1 * sizeof(uint64_t);
                
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = (uint64_t)fProvider->PPMEnabled;*/
            
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = 130;
            break;
        }
            
        //Set PPM
        case 14: {
            /*arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 0;
                
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
                
            fProvider->PPMEnabled = arguments->scalarInput[0]==1?true:false;
            
            if(!fProvider->PPMEnabled){
                fProvider->PStateCtl = 0;
                fProvider->applyPowerControl();
            }
            */
            break;
        }
            
        //Set PStateDef
        case 15: {
            /*if(!hasPrivilege())
                return kIOReturnNotPrivileged;
            
            if(arguments->scalarInputCount != 8)
                return kIOReturnBadArgument;
            
            
            fProvider->writePstate(arguments->scalarInput);
            */
            break;
        }
            
        //get board info
        case 16: {
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = fProvider->boardInfoValid ? 1 : 0;
            
            arguments->structureOutputSize = 128;

            char *dataOut = (char*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < 64; i++){
                dataOut[i] = fProvider->boardVender[i];
            }
            
            for(uint32_t i = 0; i < 64; i++){
                dataOut[i+64] = fProvider->boardName[i];
            }
            
            break;
        }
            
        default: {
            IOLog("SMCProcessorAMDUserClient::externalMethod: invalid method.\n");
            break;
        }
    }
    
    return kIOReturnSuccess;
}
