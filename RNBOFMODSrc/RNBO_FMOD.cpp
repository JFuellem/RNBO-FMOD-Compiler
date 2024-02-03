//
//  RNBO_FMOD.cpp
//  RNBO_FMOD
//
//  Created by Jonas Füllemann on 09.11.23.
//

#include <stdio.h>

#include <string.h>



#include "RNBOWrapper.hpp"


#define MAX_RNBO_PARAMETERS 1000

extern "C" {
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_dspcreate (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RNBO_dsprelease (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RNBO_dspreset (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RNBO_dspprocess (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT F_CALLBACK FMOD_RNBO_dspsetparamfloat (FMOD_DSP_STATE *dsp_state, int index, float value);
FMOD_RESULT F_CALLBACK FMOD_RNBO_dspgetparamfloat (FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_RNBO_dspsetparamdata(FMOD_DSP_STATE *dsp_state, int index, void *data, unsigned int /*length*/);
FMOD_RESULT F_CALLBACK FMOD_RNBO_dspgetparamdata(FMOD_DSP_STATE * /*dsp_state*/, int index, void ** /*value*/, unsigned int * /*length*/, char * /*valuestr*/);
FMOD_RESULT F_CALLBACK FMOD_RNBO_sys_register (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RNBO_sys_deregister (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RNBO_sys_mix (FMOD_DSP_STATE *dsp_state, int stage);

static FMOD_DSP_PARAMETER_DESC rnboParameters[MAX_RNBO_PARAMETERS];
static bool FMOD_RNBO_Running = false;
static size_t userData[10];
size_t *userDataPoints[10] = {};
FMOD_DSP_PARAMETER_DESC *FMOD_RNBO_dspparam[MAX_RNBO_PARAMETERS] = {};

enum userStorageIndex
{
    FMOD_3D_ATTR = 0,
    FMOD_TAILLENGTH,
    IS_INSTRUMENT
};


FMOD_DSP_DESCRIPTION FMOD_RNBO_Desc =
{
    FMOD_PLUGIN_SDK_VERSION,
    "JRF RNBOWrapper", //name
    0x00010000, //plugin version
    0, //inbuffer
    0, //outbuffer
    FMOD_RNBO_dspcreate,
    FMOD_RNBO_dsprelease,
    FMOD_RNBO_dspreset,
    0,
    FMOD_RNBO_dspprocess,
    0,
    MAX_RNBO_PARAMETERS,
    FMOD_RNBO_dspparam,
    FMOD_RNBO_dspsetparamfloat, //setfloat
    0, //setint
    0, //setbool
    FMOD_RNBO_dspsetparamdata, //setdata
    FMOD_RNBO_dspgetparamfloat, //getfloat
    0, //getint
    0, //getbool
    FMOD_RNBO_dspgetparamdata, //getdata
    0,
    0, //userdata
    FMOD_RNBO_sys_register,
    FMOD_RNBO_sys_deregister,
    FMOD_RNBO_sys_mix
};



extern "C"
{
F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
{
    RNBO::CoreObject obj;
    int n = (int)(obj.getNumParameters());
    int finalParameters = 0;
    RNBO::ParameterIndex i = 0;
    for(;i<n;i++)
    {
        const char *id = obj.getParameterId(i);
        //skip all subpatch params
        if (strstr(id, "/") != nullptr)
            continue;
        
        const char *name = obj.getParameterName(i);
        if(std::char_traits<char>::compare(name, "Name", 4) == 0)
        {
            strcpy(FMOD_RNBO_Desc.name, &name[5]);
        }
        else if (std::char_traits<char>::compare(name, "Sys_tail", 8) == 0)
        {
            userData[FMOD_TAILLENGTH] = obj.getParameterValue(i);
        }
        else
        {
            RNBO::ParameterInfo info;
            obj.getParameterInfo(finalParameters, &info);
            FMOD_RNBO_dspparam[finalParameters] = &rnboParameters[finalParameters];
            FMOD_DSP_INIT_PARAMDESC_FLOAT(rnboParameters[finalParameters], name, info.unit, "The RNBO param:" , info.min, info.max , info.initialValue);
            finalParameters++;
        }
    }
    
    if(CheckAnyRNBOSystemTag(obj))
    {
        FMOD_RNBO_dspparam[finalParameters] = &rnboParameters[finalParameters];
        FMOD_DSP_INIT_PARAMDESC_DATA(rnboParameters[finalParameters],"3D Attributes","","", FMOD_DSP_PARAMETER_DATA_TYPE_3DATTRIBUTES);
        
        userData[FMOD_3D_ATTR] = finalParameters;
        
        finalParameters++;
    }
    
    userData[IS_INSTRUMENT] = 1;
    
    if(obj.getNumInputChannels() > 0)
    {
        FMOD_RNBO_Desc.numinputbuffers = 1;
        userData[IS_INSTRUMENT] = 0;
    }
    
    if(obj.getNumOutputChannels() > 0)
        FMOD_RNBO_Desc.numoutputbuffers = 1;
    
    FMOD_RNBO_Desc.userdata = &userData;
    FMOD_RNBO_Desc.numparameters = finalParameters;
    
    return &FMOD_RNBO_Desc;
}

}

FMOD_RESULT F_CALLBACK FMOD_RNBO_dspcreate(FMOD_DSP_STATE *dsp_state)
{
    dsp_state->plugindata = (RNBOWrapper *)FMOD_DSP_ALLOC(dsp_state, sizeof(RNBOWrapper));
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
    RNBOWrapper *state = (RNBOWrapper *)dsp_state->plugindata;
    state->init();
    void* rawData;
    FMOD_DSP_GETUSERDATA(dsp_state, &rawData);
    state->tailLength = ((size_t*)rawData)[FMOD_TAILLENGTH];
    state->isInstrument = ((size_t*)rawData)[IS_INSTRUMENT];
    FMOD_DSP_GETSAMPLERATE(dsp_state, &state->sampleRate);
    //FMOD_DSP_LOG(dsp_state, FMOD_DEBUG_LEVEL_LOG, "Create","Samprate is %i", state->sampleRate);
    //FMOD_DSP_LOG(dsp_state, FMOD_DEBUG_LEVEL_LOG, "Create","Tail is %ims", state->tailLength);
    //FMOD_DSP_LOG(dsp_state, FMOD_DEBUG_LEVEL_LOG, "Create","Is Instrument: %i", state->isInstrument);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_dsprelease(FMOD_DSP_STATE *dsp_state)
{
    RNBOWrapper *state = (RNBOWrapper *)dsp_state->plugindata;
    FMOD_DSP_LOG(dsp_state, FMOD_DEBUG_LEVEL_LOG, "Release","%x", state);
    FMOD_DSP_FREE(dsp_state, state);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_dspprocess(FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    RNBOWrapper *state = (RNBOWrapper *)dsp_state->plugindata;
    
    auto numChans = state->rnboObj->getNumOutputChannels();
    auto numInChans = state->rnboObj->getNumInputChannels();
    
    if(op == FMOD_DSP_PROCESS_QUERY)
    {
        state->inputsIdle = inputsidle;
        
        if (!outbufferarray)
            return FMOD_ERR_DSP_DONTPROCESS;
        
        if(numInChans > 0)
            if (inbufferarray->buffernumchannels[0] != (int)numInChans)
                return FMOD_ERR_DSP_DONTPROCESS;
        
        if(inputsidle != state->lastIdleState)
        {
            state->timeStore = state->rnboObj->getCurrentTime();
            state->shouldGoIdle = false;
        }
        
        if(inputsidle && state->shouldGoIdle)
            return FMOD_ERR_DSP_DONTPROCESS;
        
        while (!state->rnboObj->prepareToProcess(state->sampleRate,4096)) {}
        
        if (outbufferarray)
        {
            outbufferarray->speakermode = GetSpeakermode(numChans);
            outbufferarray->buffernumchannels[0] = (int)numChans;
        }
        
        state->lastIdleState =  inputsidle;
        
        return FMOD_OK;
    }
#ifdef __APPLE__
    state->rnboObj->process<float*, float*>(inbufferarray[0].buffers[0], numInChans, outbufferarray[0].buffers[0], numChans, length);
#elif _WIN64
    state->rnboObj->process<float*>(inbufferarray[0].buffers[0], numInChans, outbufferarray[0].buffers[0], numChans, length);
#endif
    if(!state->isInstrument && state->inputsIdle)
    {
        if (state->tailLength <= 0)
        {
            state->shouldGoIdle = true;
            return FMOD_OK;
        }
            
        if (CheckIfOutputQuiet(outbufferarray[0].buffers[0], length, numChans))
        {
            state->shouldGoIdle = (state->rnboObj->getCurrentTime() - state->timeStore) > state->tailLength;
        }
        else
        {
            state->timeStore = state->rnboObj->getCurrentTime();
        }
    }
    /*
    static size_t counter = 0;
    if (counter % 500 == 0)
        FMOD_DSP_LOG(dsp_state, FMOD_DEBUG_LEVEL_LOG, "Proc","Idle: %i", inputsidle);
    counter++;
    */
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK FMOD_RNBO_dspreset(FMOD_DSP_STATE *dsp_state)
{
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_dspsetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float value)
{
    
    RNBOWrapper *state = (RNBOWrapper *)dsp_state->plugindata;
    
    if(index < state->rnboObj->getNumParameters())
    {
        state->rnboObj->setParameterValue(index, value);
        return FMOD_OK;
    }
    
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_dspgetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr)
{
    RNBOWrapper *state = (RNBOWrapper *)dsp_state->plugindata;
    
    if(index < state->rnboObj->getNumParameters())
    {
        *value = state->rnboObj->getParameterValue(index);
        return FMOD_OK;
    }
    
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_dspsetparamdata(FMOD_DSP_STATE *dsp_state, int index, void *data, unsigned int /*length*/)
{
    RNBOWrapper *state = (RNBOWrapper *)dsp_state->plugindata;
    void* indexptr;
    FMOD_DSP_GETUSERDATA(dsp_state, &indexptr);
    size_t *posIndex = static_cast<size_t*>(indexptr);
    
    FMOD_DSP_LOG(dsp_state, FMOD_DEBUG_LEVEL_LOG, "DataOutside Index: ","%i",index);
    FMOD_DSP_LOG(dsp_state, FMOD_DEBUG_LEVEL_LOG, "DataOutside User: ","%i", posIndex[0]);

    if(index == posIndex[FMOD_3D_ATTR])
    {
        DispatchRNBOMessages(state->rnboObj.get(), data);
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_dspgetparamdata(FMOD_DSP_STATE *dsp_state, int index, void ** /*value*/, unsigned int * /*length*/, char * /*valuestr*/)
{
    void* indexptr;
    FMOD_DSP_GETUSERDATA(dsp_state, &indexptr);
    size_t *posIndex = static_cast<size_t*>(indexptr);
    
    if(index == posIndex[FMOD_3D_ATTR])
        return FMOD_ERR_INVALID_PARAM;

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_sys_register(FMOD_DSP_STATE * /*state*/)
{
    FMOD_RNBO_Running = true;
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_sys_deregister(FMOD_DSP_STATE * /*state*/)
{
    FMOD_RNBO_Running = false;
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RNBO_sys_mix(FMOD_DSP_STATE * /*state*/, int /*stage*/)
{
    return FMOD_OK;
}



