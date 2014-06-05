#ifndef INCLUDE_AL_PANNING_DBAP
#define INCLUDE_AL_PANNING_DBAP

#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/math/al_Quat.hpp"
//#include "allocore/spatial/al_CoordinateFrame.hpp"

namespace al{

#define DBAP_MAX_NUM_SPEAKERS 192
#define DBAP_MAX_DIST 100

class Dbap : public Spatializer{
public:
	
    Dbap(SpeakerLayout &sl, float _spread = 5.f) : Spatializer(sl), numSpeakers(0), spread(_spread){}
    
	void dump() {
		printf("Using DBAP Panning- need to add panner info for dump function\n");
	}
		
	void compile(Listener& listener){
		mListener = &listener;
		numSpeakers = mSpeakers.size();
		printf("DBAP Compiled with %d speakers\n", numSpeakers);
		
		for(int i = 0; i < numSpeakers; i++)
		{
			speakerVecs[i] = mSpeakers[i].vec();
			speakerVecs[i].normalize();
			deviceChannels[i] = mSpeakers[i].deviceChannel;
		}
			
	}
	
    ///Per Sample Processing
    void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample)
    {
		for (unsigned i = 0; i < numSpeakers; ++i)
        {
            Vec3d vec = relpos.normalized();
            vec -= speakerVecs[i];
			float dist = vec.mag() / 2.f; // [0, 1]
            dist = powf(dist, spread);
            float gain = 1.f / (1.f + DBAP_MAX_DIST*dist);
            
            io.out(deviceChannels[i],frameIndex) += gain*sample;
		}
	}
    
    /// Per Buffer Processing
	void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples)
    {
		for (unsigned i = 0; i < numSpeakers; ++i)
        {
            Vec3d vec = relpos.normalized();
            vec -= speakerVecs[i];
            float dist = vec.mag() / 2.f; // [0, 1]
            dist = powf(dist, spread);
            float gain = 1.f / (1.f + DBAP_MAX_DIST*dist);
            
            float *buf = io.outBuffer(deviceChannels[i]);
			float *samps = samples;
            for(int j = 0; j < numFrames; j++)
				*buf++ += gain* *samps++;
		}
	}
	
	
	//    for(int c = 0; c < io.channelsOut(); c++)
	//    {
	//        float *buf = io.outBuffer(c);
	//        float *subbuf = subBuffer;
	//        for(int i = 0; i < io.framesPerBuffer(); i++)
	//        {
	//			*buf++ = MASTER_GAIN* *subbuf++;
	//        }
	//    }
	
private:
	Listener* mListener;
	Vec3f speakerVecs[DBAP_MAX_NUM_SPEAKERS];
	int deviceChannels[DBAP_MAX_NUM_SPEAKERS];
	int numSpeakers;
    float spread;
};
	
	

} // al::

#endif
