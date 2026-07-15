//
//  SMCProcessorAMD
//

#include "KeyImplementations.hpp"


SMC_RESULT TempPackage::readAccess() {
    uint16_t *ptr = reinterpret_cast<uint16_t *>(data);
    *ptr = VirtualSMCAPI::encodeSp(type, (double)provider->PACKAGE_TEMPERATURE_perPackage[0]);

    return SmcSuccess;
}

SMC_RESULT TempCore::readAccess() {
    uint16_t *ptr = reinterpret_cast<uint16_t *>(data);
    *ptr = VirtualSMCAPI::encodeSp(type, (double)provider->PACKAGE_TEMPERATURE_perPackage[0]);

    return SmcSuccess;
}

SMC_RESULT EnergyPackage::readAccess(){
    if (type == SmcKeyTypeFloat)
        *reinterpret_cast<uint32_t *>(data) = VirtualSMCAPI::encodeFlt(provider->uniPackageEnergy);
    else
        *reinterpret_cast<uint16_t *>(data) = VirtualSMCAPI::encodeSp(type, provider->uniPackageEnergy);
    
    return SmcSuccess;
}

// Per-core clock speed in MHz, decoded from the P-state MSR in
// SMCProcessorAMD::updateClockSpeed() and cached in clockSpeed_perCore[].
SMC_RESULT FreqCore::readAccess() {
    float freq = 0.0f;
    if (provider) {
        uint32_t maxCore = provider->totalNumberOfPhysicalCores;
        if (maxCore > CPUInfo::MaxCpus) maxCore = CPUInfo::MaxCpus;
        if (core < maxCore) {
            freq = provider->clockSpeed_perCore[core];
        }
    }

    if (type == SmcKeyTypeFloat)
        *reinterpret_cast<uint32_t *>(data) = VirtualSMCAPI::encodeFlt(freq);
    else
        *reinterpret_cast<uint16_t *>(data) = VirtualSMCAPI::encodeSp(type, (double)freq);

    return SmcSuccess;
}

// Package-wide average clock speed in MHz across all physical cores currently
// reporting a non-zero frequency.
SMC_RESULT FreqPackage::readAccess() {
    float freq = 0.0f;
    if (provider) {
        uint32_t count = provider->totalNumberOfPhysicalCores;
        if (count > CPUInfo::MaxCpus) count = CPUInfo::MaxCpus;

        float sum = 0.0f;
        uint32_t validCount = 0;
        for (uint32_t i = 0; i < count; i++) {
            float coreFreq = provider->clockSpeed_perCore[i];
            if (coreFreq > 0.0f) {
                sum += coreFreq;
                validCount++;
            }
        }
        if (validCount > 0) freq = sum / (float)validCount;
    }

    if (type == SmcKeyTypeFloat)
        *reinterpret_cast<uint32_t *>(data) = VirtualSMCAPI::encodeFlt(freq);
    else
        *reinterpret_cast<uint16_t *>(data) = VirtualSMCAPI::encodeSp(type, (double)freq);

    return SmcSuccess;
}
