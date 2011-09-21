#ifndef AL_WORLD_HPP_INC
#define AL_WORLD_HPP_INC
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
	Base class for common applications

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <math.h>
#include <string>
#include <vector>

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"

//#include "GLV/glv.h"

namespace al{


/// Viewpoint within a scene

/// A viewpoint is an aggregation of a viewport (screen region), a pose
/// (3D position and orientation), and a camera.
class Viewpoint{
public:

	Viewpoint(const Pose& transform = Pose::identity())
	:	mViewport(0,0,0,0),
		mParentTransform(0),
		mAnchorX(0), mAnchorY(0), mStretchX(1), mStretchY(1),
		mCamera(0), mClearColor(0)
	{}

	float anchorX() const { return mAnchorX; }
	float anchorY() const { return mAnchorY; }
	float stretchX() const { return mStretchX; }
	float stretchY() const { return mStretchY; }

	/// Set anchoring factors relative to bottom-left corner of window
	
	/// @param[in] ax	anchor factor relative to left edge of window, in [0,1]
	/// @param[in] ay	anchor factor relative to bottom edge of window, in [0,1]
	Viewpoint& anchor(float ax, float ay){
		mAnchorX=ax; mAnchorY=ay; return *this;
	}

	/// Set stretch factors relative to bottom-left corner of window
	
	/// @param[in] sx	stretch factor relative to left edge of window, in [0,1]
	/// @param[in] sy	stretch factor relative to bottom edge of window, in [0,1]
	Viewpoint& stretch(float sx, float sy){
		mStretchX=sx; mStretchY=sy; return *this;
	}

	bool hasCamera() const { return 0 != mCamera; }
	bool hasClearColor() const { return 0 != mClearColor; }

	const Camera& camera() const { return *mCamera; }
	Viewpoint& camera(Camera& v){ mCamera=&v; return *this; }
	
	const Color& clearColor() const { return *mClearColor; }
	Viewpoint& clearColor(Color& v){ mClearColor=&v; return *this; }

	const Pose* parentTransform() const { return mParentTransform; }
	Viewpoint& parentTransform(Pose& v){ mParentTransform = &v; return *this; }

	const Pose& transform() const { return mTransform; }
	Pose& transform(){ return mTransform; }
	
	Pose worldTransform() const { return mParentTransform ? (*mParentTransform) * transform() : transform(); }
	
	const Viewport& viewport() const { return mViewport; }
	Viewport& viewport(){ return mViewport; }

protected:
	Viewport mViewport;				// screen display region
	Pose * mParentTransform;		// parent transform, 0 if none
	Pose mTransform;				// local transform
	float mAnchorX, mAnchorY;		// viewport anchor factors relative to parent window
	float mStretchX, mStretchY;		// viewport stretch factors relative to parent window
	Camera * mCamera;				// camera; if not set, will be set to scene's default camera
	Color * mClearColor;
};


/// A window with one or more Viewpoints
class ViewpointWindow : public Window {
public:
	using Window::add;
	typedef std::vector<Viewpoint *> Viewpoints;

	ViewpointWindow(){
		init();
	}
	
	ViewpointWindow(
		const Dim& dims,
		const std::string title="",
		double fps=40,
		DisplayMode::t mode = DisplayMode::DefaultBuf
	){
		init();
		create(dims, title, fps, mode);
	}

	const Viewpoints& viewpoints() const { return mViewpoints; }
	
	ViewpointWindow& add(Viewpoint& v){ mViewpoints.push_back(&v); return *this; }

protected:
	Viewpoints mViewpoints;

	virtual bool onResize(int dw, int dh);

private:
	void init(){
		add(new StandardWindowKeyControls);
	}
};


/// Application helper class
class App {
public:

	typedef std::vector<Listener *>			Listeners;
	typedef std::vector<ViewpointWindow *>	Windows;

	App();
	virtual ~App();


	/// Initialize audio
	void initAudio(
		double audioRate=44100, int audioBlockSize=128,
		int audioOutputs=-1, int audioInputs=-1	
	);
	
	
	/// Initialize a new window
	ViewpointWindow * initWindow(
		const Window::Dim& dims = Window::Dim(800,600),
		const std::string title="",
		double fps=40,
		DisplayMode::t mode = DisplayMode::DefaultBuf,
		int flags=0
	);

	/// Start rendering; begins audio and drawing callbacks
	void start();


	/// Sound generation callback

	/// Sound can either be written directly to the audio output channels
	/// or to the sound source's internal buffer which is rendered later by the
	/// spatial audio decoder.
	virtual void onSound(AudioIOData& io){}

	/// Animation (model update) callback
	virtual void onAnimate(double dt){}

	/// Drawing callback (in world coordinates)
	
	/// This will be called from the main graphics renderer. Since it may be 
	/// called multiple times, no state updates should be made in it.
	virtual void onDraw(Graphics& g, const Viewpoint& v){}

	virtual void onCreate(const ViewpointWindow& win){}
	virtual void onDestroy(const ViewpointWindow& win){}


	const AudioIO&		audioIO() const { return mAudioIO; }
	AudioIO&			audioIO(){ return mAudioIO; }

	//const AudioScene&	audioScene() const { return mAudioScene; }
	//AudioScene&			audioScene(){ return mAudioScene; }

	const Camera&		camera() const { return mCamera; }
	Camera&				camera(){ return mCamera; }

	const Graphics&		graphics() const { return mGraphics; }
	Graphics&			graphics(){ return mGraphics; }

	const std::string&	name() const { return mName; }
	App&				name(const std::string& v){ mName=v; return *this; }

	const Nav&			nav() const { return mNav; }
	Nav&				nav(){ return mNav; }

	const Nav&			navDraw() const { return mNavDraw; }
	Nav&				navDraw(){ return mNavDraw; }

	const NavInputControl& navControl() const { return mNavControl; }
	NavInputControl&	navControl(){ return mNavControl; }

	osc::Recv&			oscRecv(){ return mOSCRecv; }
	osc::Send&			oscSend(){ return mOSCSend; }

	const Stereographic& stereo() const { return mStereo; }
	Stereographic&		stereo(){ return mStereo; }

	const Windows&		windows() const { return mWindows; }
	Windows&			windows(){ return mWindows; }

	const void *		clockAnimate() const { return mClockAnimate; }
	App&				clockAnimate(void * v){ mClockAnimate=v; return *this; }

	const void *		clockNav() const { return mClockNav; }
	App&				clockNav(void * v){ mClockNav=v; return *this; }

	/// Add a window to the world
	
	/// @param[in] win			The window to add
	///
	App& add(ViewpointWindow& win);


	void sendHandshake(){
		oscSend().send("/handshake", name(), oscRecv().port());
	}
	
	void sendDisconnect(){
		oscSend().send("/disconnectApplication", name());
	}

private:

	typedef std::vector<Viewpoint> Viewpoints;

	Viewpoints mFacViewpoints;
	Windows mFacWindows;

	// graphics
	Windows mWindows;
	Camera mCamera;
	Stereographic mStereo;
	Graphics mGraphics;
	
	// sound
	AudioIO mAudioIO;
	//AudioScene mAudioScene;
	//Listeners mListeners;

	// spatial
	Nav mNav;
	Nav mNavDraw;	// this version remains invariant throughout all drawing
	
	// control
	NavInputControl mNavControl;
	osc::Recv mOSCRecv;
	osc::Send mOSCSend;

	std::string mName;
	void * mClockAnimate;
	void * mClockNav;

	bool usingAudio() const;

	// attached to each ViewpointWindow
	struct SceneWindowHandler : public WindowEventHandler{

		ViewpointWindow& win;
		App& app;

		SceneWindowHandler(ViewpointWindow& w, App& a): win(w), app(a){}
	
		virtual bool onCreate(){
			app.onCreate(win);
			return true;
		}

		virtual bool onDestroy(){
			app.onDestroy(win);
			return true;
		}
	
		virtual bool onFrame();
	};
	
	struct SceneInputHandler : public InputEventHandler{
		SceneInputHandler(App& a): app(a){}
		
		virtual bool onKeyDown(const Keyboard& k){
			switch(k.key()){
				case Key::Tab: app.stereo().stereo(!app.stereo().stereo()); return false;
				default:;
			}
			return true;
		}

		App& app;
	};

};

} // al::
#endif