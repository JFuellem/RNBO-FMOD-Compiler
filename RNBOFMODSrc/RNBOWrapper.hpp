//
//  RNBOWrapper.hpp
//  RNBO_FMOD
//
//  Created by Jonas Füllemann on 19.01.24.
//

#ifndef RNBOWrapper_hpp
#define RNBOWrapper_hpp

#include <stdio.h>
#include <format>
#include <vector>

#include "RNBO.h"
#include "fmod.hpp"


#define RNBOObj RNBO::CoreObject

class RNBOWrapper
{
public:
    std::unique_ptr<RNBOObj> rnboObj;
    FMOD_BOOL inputsIdle = 0;
    FMOD_BOOL lastIdleState = 0;
    bool shouldGoIdle = false;
    size_t timeStore = 0;
    size_t tailLength = 0;
    bool isInstrument = false;
    int sampleRate = 48000;
    void init()
    {
        rnboObj = std::make_unique<RNBOObj>();
    }
    void reset()
    {
        shouldGoIdle = false;
        inputsIdle = 0;
    }
};

static bool CheckRNBOTag(const RNBOObj &obj, const char* tag)
{
    return strcmp(obj.resolveTag(RNBO::TAG(tag)), tag) == 0;
}

static inline bool CheckAnyRNBOSystemTag(const RNBOObj &obj)
{
    return (CheckRNBOTag(obj, "rel_pos") || CheckRNBOTag(obj, "rel_vel") ||
            CheckRNBOTag(obj, "rel_forw") || CheckRNBOTag(obj, "rel_up") ||
            CheckRNBOTag(obj, "abs_pos") || CheckRNBOTag(obj, "abs_vel") ||
            CheckRNBOTag(obj, "abs_forw") || CheckRNBOTag(obj, "abs_up"));
}

static inline void DispatchRNBOMessages(RNBOObj *obj, void* rawData)
{
    FMOD_DSP_PARAMETER_3DATTRIBUTES* param = (FMOD_DSP_PARAMETER_3DATTRIBUTES*)rawData;
    if(CheckRNBOTag(*obj, "rel_pos"))
    {
        auto posList = RNBO::make_unique<RNBO::list>();
        posList->push(param->relative.position.x);
        posList->push(param->relative.position.y);
        posList->push(param->relative.position.z);
        obj->sendMessage(RNBO::TAG("rel_pos"), std::move(posList), RNBO::TAG(""));
    }
    if(CheckRNBOTag(*obj, "rel_vel"))
    {
        auto posList = RNBO::make_unique<RNBO::list>();
        posList->push(param->relative.velocity.x);
        posList->push(param->relative.velocity.y);
        posList->push(param->relative.velocity.z);
        obj->sendMessage(RNBO::TAG("rel_vel"), std::move(posList), RNBO::TAG(""));
    }
    if(CheckRNBOTag(*obj, "rel_forw"))
    {
        auto posList = RNBO::make_unique<RNBO::list>();
        posList->push(param->relative.forward.x);
        posList->push(param->relative.forward.y);
        posList->push(param->relative.forward.z);
        obj->sendMessage(RNBO::TAG("rel_forw"), std::move(posList), RNBO::TAG(""));
    }
    if(CheckRNBOTag(*obj, "rel_up"))
    {
        auto posList = RNBO::make_unique<RNBO::list>();
        posList->push(param->relative.up.x);
        posList->push(param->relative.up.y);
        posList->push(param->relative.up.z);
        obj->sendMessage(RNBO::TAG("rel_up"), std::move(posList), RNBO::TAG(""));
    }
    if(CheckRNBOTag(*obj, "abs_pos"))
    {
        auto posList = RNBO::make_unique<RNBO::list>();
        posList->push(param->absolute.position.x);
        posList->push(param->absolute.position.y);
        posList->push(param->absolute.position.z);
        obj->sendMessage(RNBO::TAG("abs_pos"), std::move(posList), RNBO::TAG(""));
    }
    if(CheckRNBOTag(*obj, "abs_vel"))
    {
        auto posList = RNBO::make_unique<RNBO::list>();
        posList->push(param->absolute.velocity.x);
        posList->push(param->absolute.velocity.y);
        posList->push(param->absolute.velocity.z);
        obj->sendMessage(RNBO::TAG("abs_vel"), std::move(posList), RNBO::TAG(""));
    }
    if(CheckRNBOTag(*obj, "abs_forw"))
    {
        auto posList = RNBO::make_unique<RNBO::list>();
        posList->push(param->absolute.forward.x);
        posList->push(param->absolute.forward.y);
        posList->push(param->absolute.forward.z);
        obj->sendMessage(RNBO::TAG("abs_forw"), std::move(posList), RNBO::TAG(""));
    }
    if(CheckRNBOTag(*obj, "abs_up"))
    {
        auto posList = RNBO::make_unique<RNBO::list>();
        posList->push(param->absolute.up.x);
        posList->push(param->absolute.up.y);
        posList->push(param->absolute.up.z);
        obj->sendMessage(RNBO::TAG("abs_up"), std::move(posList), RNBO::TAG(""));
    }
}



static inline FMOD_SPEAKERMODE GetSpeakermode(const RNBO::Index& channels)
{
    switch(channels)
    {
        case 1:
            return FMOD_SPEAKERMODE_MONO;
            break;
        case 2:
            return FMOD_SPEAKERMODE_STEREO;
            break;
        case 4:
            return FMOD_SPEAKERMODE_QUAD;
            break;
        case 5:
            return FMOD_SPEAKERMODE_SURROUND;
            break;
        case 6:
            return FMOD_SPEAKERMODE_5POINT1;
            break;
        case 8:
            return FMOD_SPEAKERMODE_7POINT1;
            break;
        case 12:
            return FMOD_SPEAKERMODE_7POINT1POINT4;
            break;
        default:
            return FMOD_SPEAKERMODE_DEFAULT;
            break;
    }
}

static inline bool CheckIfOutputQuiet(float* outarray,size_t buffsize, size_t numchans)
{
    buffsize *= numchans;
    for (size_t i = 0; i< buffsize;i++)
    {
        if (abs(outarray[i]) > 1e-9)
            return false;
    }
    return true;
}

#endif /* RNBOWrapper_hpp */
