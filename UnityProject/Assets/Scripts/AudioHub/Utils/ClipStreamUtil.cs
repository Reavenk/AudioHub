using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ClipStreamUtil
{
    public const int sampleRate = 16000;
    public const int bufferSize = sampleRate;

    AudioClip micRecording;
    private int lastRecordPos = 0;

    public string DeviceName {get{ return this.deviceName; } }
    private string deviceName = string.Empty;

    public bool Start(int device = 0)
    { 
        if(device < 0 || device >= Microphone.devices.Length)
            return false;

        this.deviceName = Microphone.devices[device];
        this.micRecording = Microphone.Start(this.deviceName, true, 1, bufferSize);
        this.lastRecordPos = 0;
        return true;
    }

    public bool Stop()
    { 
        if(!this.micRecording)
            return false;

        Microphone.End(this.deviceName);
        this.deviceName = "";
        GameObject.Destroy(this.micRecording);
        this.micRecording = null;
        this.lastRecordPos = 0;
        return true;
    }

    public bool IsStreaming()
    { 
        return !string.IsNullOrEmpty(this.deviceName);
    }

    /// <summary>
    /// Since the last line GetNewPCM() was called, get the newest 
    /// streaming microphone as a signed short array.
    /// </summary>
    public short[] GetNewPCM()
    { 
        if(!this.micRecording)
            return new short[]{ };

        int oldRecPos = this.lastRecordPos;
        int newRecPos = Microphone.GetPosition(this.deviceName);
        this.lastRecordPos = newRecPos;

        if(newRecPos == oldRecPos)
            return new short[]{ };

        // If the new position is less than the old, we've wrapped around
        // in the toroidal array.
        // In which case, we need to combine the start and end into a single
        // return.
        if(newRecPos < oldRecPos)
        {
            int totalClipSamples = this.micRecording.samples;
            int endWrapSamples = totalClipSamples - oldRecPos;
            int totalWrapSamps = endWrapSamples + newRecPos;
            short[] retWrapPCM = new short[totalWrapSamps];
            // Get the buffer end samples
            float [] rfWrap = new float[endWrapSamples];
            this.micRecording.GetData(rfWrap, totalWrapSamps - endWrapSamples);
            for(int i = 0; i < endWrapSamples; ++i)
                retWrapPCM[i] = (short)(rfWrap[i] * short.MaxValue);

            // Get the buffer wrap-around samples
            if(newRecPos != 0)
            {
                rfWrap = new float[newRecPos];
                this.micRecording.GetData(rfWrap, 0);
                
                for(int i = 0; i < newRecPos; ++i)
                    retWrapPCM[endWrapSamples + i] = (short)(rfWrap[i]* short.MaxValue);
            }

            return retWrapPCM;
        }

        // If we just need to convert a simple subarray.
        int totalSamples = newRecPos - oldRecPos;
        short[] retPCM = new short[totalSamples];
        float[] rf = new float[totalSamples];
        this.micRecording.GetData(rf, oldRecPos);
        for (int i = 0; i < totalSamples; ++i)
            retPCM[i] = (short)(rf[i] * short.MaxValue);

        return retPCM;
    }
}
