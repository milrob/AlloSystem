#ifndef INCLUDE_AL_REVERB_HPP
#define INCLUDE_AL_REVERB_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

	File description:
	Mono-to-stereo reverberator

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <stdlib.h>
#include <strings.h>

namespace al{


/// Delay-line whose maximum size is fixed

/// The advantage of using a static versus dynamic array is that its elements
/// can be laid out in a predictable location in memory. This can improve
/// access speeds if many delay-lines are used within another object, like a
/// reverb.
template <int N, class T>
class StaticDelayLine {
public:

	StaticDelayLine(): mPos(0){ zero(); }
	

	/// Get size of delay-line
	static int size(){ return N; }

	/// Get element at back
	const T& back() const { return mBuf[indexBack()]; }

	/// Get index of back element
	int indexBack() const {
		int i = pos()+1;
		return (i < size()) ? i : 0;
	}

	/// Get absolute index of write tap
	int pos() const { return mPos; }


	/// Read value at delay i
	const T& read(int i) const {
		int ind = pos()-i;
		if(ind < 0) ind += size();
		//else if(ind >= size()) ind -= size();
		return mBuf[ind];
	}
	
	/// Write value to delay
	void write(const T& v){
		mBuf[pos()] = v;
		++mPos; if(mPos >= size()) mPos=0;
	}
	
	/// Write new value and return oldest value
	T operator()(const T& v){
		T r = mBuf[pos()];
		write(v);
		return r;
	}
	
	/// Comb filter input using a delay time equal to the maximum size of the delay-line
	T comb(const T& v, const T& ffd, const T& fbk){
		T d = mBuf[pos()];
		T r = v + d*fbk;
		write(r);
		return d + r*ffd;
	}
	
	/// Zeroes all elements (byte-wise)
	void zero(){ ::bzero(&mBuf, sizeof(mBuf)); }

protected:
	int mPos;
	T mBuf[N];
};



/// Plate reverberator

/// Design from:
/// Dattorro, J. (1997). Effect design: Part 1: Reverberator and other filters. 
/// Journal of the Audio Engineering Society, 45(9):660–684.
/// https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf
template <class T = float>
class Reverb{
public:

	Reverb(){
//		bandwidth(0.9995);
//		decay(0.5);
//		damping(0.0005);
//		diffusion(0.75, 0.625, 0.7, 0.5);
		bandwidth(0.9995);
		decay(0.85);
		damping(0.4);
		diffusion(0.76, 0.666, 0.707, 0.571);
	}

	/// Set input signal bandwidth, [0,1]
	Reverb& bandwidth(T v){ mOPIn.damping(T(1)-v); return *this; }

	/// Set high-frequency damping amount, [0, 1]
	Reverb& damping(T v){ mOP1.damping(v); mOP2.damping(v); return *this; }

	/// Set decay rate, [0, 1)
	Reverb& decay(T v){ mDecay=v; return *this; }

	/// Set diffusion amounts
	
	/// The recommended range of these coefficients is from 0.0 to 0.9999999
	///
	Reverb& diffusion(T in1, T in2, T decay1, T decay2){
		mDfIn1=in1;	mDfIn2=in2; mDfDcy1=decay1; mDfDcy2=decay2;
		return *this;
	}
	
	/// Set input diffusion 1 amount, [0,1)
	Reverb& diffusionIn1(const T& v){ mDfIn1=v; return *this; }
	
	/// Set input diffusion 2 amount, [0,1)
	Reverb& diffusionIn2(const T& v){ mDfIn2=v; return *this; }
	
	/// Set tank decay diffusion 1 amount, [0,1)
	Reverb& diffusionDecay1(const T& v){ mDfDcy1=v; return *this; }
	
	/// Set tank decay diffusion 2 amount, [0,1)
	Reverb& diffusionDecay2(const T& v){ mDfDcy2=v; return *this; }


	/// Compute wet stereo output from dry mono input
	
	/// @param[ in] in		dry input sample
	/// @param[out] out1	wet output sample 1
	/// @param[out] out2	wet output sample 2
	/// @param[ in] gain	gain of output
	void operator()(const T& in, T& out1, T& out2, T gain = T(0.6)){
		T v = mPreDelay(in * T(0.5));
		v = mOPIn(v);
		v = mAPIn1.comb(v, mDfIn1,-mDfIn1);
		v = mAPIn2.comb(v, mDfIn1,-mDfIn1);
		v = mAPIn3.comb(v, mDfIn2,-mDfIn2);
		v = mAPIn4.comb(v, mDfIn2,-mDfIn2);
		
		T a = v + mDly22.back() * mDecay;
		T b = v + mDly12.back() * mDecay;
		
		a = mAPDecay11.comb(a,-mDfDcy1, mDfDcy1);
		a = mDly11(a);
		a = mOP1(a) * mDecay;
		a = mAPDecay12.comb(a, mDfDcy2,-mDfDcy2);
		mDly12.write(a);

		b = mAPDecay21.comb(b,-mDfDcy1, mDfDcy1);
		b = mDly21(b);
		b = mOP2(b) * mDecay;
		b = mAPDecay22.comb(b, mDfDcy2,-mDfDcy2);
		mDly22.write(b);
		
		out1 = (  mDly21.read(266)
				+ mDly21.read(2974)
				- mAPDecay22.read(1913)
				+ mDly22.read(1996)
				- mDly11.read(1990)
				- mAPDecay12.read(187)
				- mDly12.read(1066)) * gain;

		out2 = (  mDly11.read(353)
				+ mDly11.read(3627)
				- mAPDecay12.read(1228)
				+ mDly12.read(2673)
				- mDly21.read(2111)
				- mAPDecay22.read(335)
				- mDly22.read(121)) * gain;
	}

	/// Compute wet/dry mix stereo output from dry mono input
	
	/// @param[inout] inout1		the input sample and wet/dry output 1
	/// @param[  out]   out2		wet/dry output 2
	/// @param[in   ] wetAmt		wet mix amount
	/// \returns dry input sample
	T mix(T& inout1, T& out2, T wetAmt){
		T s = inout1;
		(*this)(s, inout1, out2, wetAmt*T(0.6));
		inout1 += s;
		  out2 += s;
		return s;
	}

protected:

	class OnePole{
	public:
		OnePole(): mO1(0), mB1(0){}
		void damping(const T& v){ mB1=v; }
		T operator()(const T& i0){ return mO1 = (mO1-i0)*mB1 + i0; }
	protected:
		T mO1; // previous output
		T mB1; // previous output coef
	};

	T mDfIn1, mDfIn2, mDfDcy1, mDfDcy2, mDecay;

	StaticDelayLine<  10,T> mPreDelay;
	OnePole mOPIn;
	StaticDelayLine< 142,T> mAPIn1;
	StaticDelayLine< 107,T> mAPIn2;
	StaticDelayLine< 379,T> mAPIn3;
	StaticDelayLine< 277,T> mAPIn4;
	StaticDelayLine< 672,T> mAPDecay11;
	StaticDelayLine<1800,T> mAPDecay12;
	StaticDelayLine<4453,T> mDly11;
	StaticDelayLine<3720,T> mDly12;
	OnePole mOP1;
	StaticDelayLine< 908,T> mAPDecay21;
	StaticDelayLine<2656,T> mAPDecay22;
	StaticDelayLine<4217,T> mDly21;
	StaticDelayLine<3163,T> mDly22;
	OnePole mOP2;
};

} // al::
#endif
