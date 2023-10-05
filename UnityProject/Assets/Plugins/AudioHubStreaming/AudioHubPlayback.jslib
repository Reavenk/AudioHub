mergeInto(LibraryManager.library, 
{

	AH_ReportPlaybackFloatSamples: function (heapPtr) 
	{ 
		StartMic(); 
	},
	AH_PlaybackStart: function() 
	{ 
	},
	AH_PlaybackStop: function() 
	{ 
	},
	AH_PlaybackInitialize : function()
	{
		if(window.ahPlaybackContent)
			throw new Error("AudioHub playback context already unexpectedly exists")
			
		const audioContext = new AudioContext();
		window.ahPlaybackContent = audioContext
		const audioSource = audioContext.createBufferSource();
		const audioBuffer = audioContext.createBuffer(1, 16000, 16000)
	},
	AH_PlaybackShutdown : function()
	{
		if(!window.ahPlaybackContent)
			throw new Error("Missing expected AudioHub playback context");
		
		window.ahPlaybackContent.close();
		window.ahPlaybackContent = null;
	}
})