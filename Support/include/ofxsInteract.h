#ifndef _ofxsInteract_H_
#define _ofxsInteract_H_
/*
  OFX Support Library, a library that skins the OFX plug-in API with C++ classes.
  Copyright (C) 2004-2005 The Foundry Visionmongers Ltd
  Author Bruno Nicoletti bruno@thefoundry.co.uk

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name The Foundry Visionmongers Ltd, nor the names of its 
      contributors may be used to endorse or promote products derived from this
      software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The Foundry Visionmongers Ltd
1 Wardour St
London W1D 6PA
England



*/

/** @file This file contains core code that wraps OFX 'objects' with C++ classes.

This file only holds code that is visible to a plugin implementation, and so hides much
of the direct OFX objects and any library side only functions.
 */
#include "ofxsParam.h"

#include <list>

/** @brief Nasty macro used to define empty protected copy ctors and assign ops */
#define mDeclareProtectedAssignAndCC(CLASS) \
  CLASS &operator=(const CLASS &v1) {assert(false); return *this;}      \
  CLASS(const CLASS &v) {assert(false); } 

/** @brief The core 'OFX Support' namespace, used by plugin implementations. All code for these are defined in the common support libraries.
 */
namespace OFX {

    /** @brief forward declaration */
    class ImageEffect;

    /// all image effect interacts have these argumens
    struct InteractArgs {
       /// ctor
       InteractArgs(const PropertySet &props);
       double          time;              /**< @brief The current effect time to draw at */
       OfxPointD       renderScale;       /**< @brief The current render scale being applied to any image that would be fetched */
    };

    /** @brief struct to pass arguments into OFX::Interact::draw */
    struct DrawArgs : public InteractArgs {
        DrawArgs(const PropertySet &props);

        OfxPointD       pixelScale;        /**< @brief The current effect time to draw at */
        OfxRGBColourD   backGroundColour;  /**< @brief The current background colour, ignore the A */
    };

    /** @brief POD  to pass arguments into OFX::Interact pen actions */
    struct PenArgs : public InteractArgs {
        PenArgs(const PropertySet &props);

        OfxPointD       pixelScale;        /**< @brief The current effect time to draw at */
        OfxPointD       penPosition;       /**< @brief The current pen position */
        double          penPressure;       /**< @brief The normalised pressure on the pen */
    };

    /** @brief  struct to pass arguments into OFX::Interact key actions 

    Note 
      - some keys cannot be represented as UTF8 strings (eg: the key pad page down key kOfxKey_KP_Page_Up), in which case the key string will be set to "".
      - some UTF8 symbols (generally non-English language ones) cannot be represented by one of the keySymbols, in which case the UTF8 string will be set to some non empty value, but the keySymbol will be set to kOfxKey_Unknown.
      - in no case will keyString be set to "" and keySymbol be set to kOfxKey_Unknown.
    */
    struct KeyArgs : public InteractArgs {
        KeyArgs(const PropertySet &props);

        int             keySymbol;         /**< @brief The key represented as one of the entries in ofxKeySyms.h, see note in OFX::KeyArgs */
        std::string     keyString;         /**< @brief That key as a UTF8 string, see note in OFX::KeyArgs */
    };

    /** @brief struct to pass arguments into OFX::Interact focus actions */
    struct FocusArgs : public InteractArgs {
        FocusArgs(const PropertySet &props);

        OfxPointD       pixelScale;        /**< @brief The current effect time to draw at */
        OfxRGBColourD   backGroundColour;  /**< @brief The current background colour, ignore the A */
    };

    /** @brief Wraps up an OFX interact object for an Image Effect. It won't work for any other plug-in type at present (it would need to be broken into a hierarchy of classes).
    */
    class Interact {
    private :
        OfxInteractHandle  _interactHandle;     /**< @brief The handle for this interact */
        PropertySet        _interactProperties; /**< @brief The property set on this interact */
        std::list<Param *> _slaveParams;        /**< @brief List of params we are currently slaved to */

    protected :
        ImageEffect *_effect;                   /**< @brief The instance we are associated with */

    public : 
        /** @brief ctor */
        Interact(OfxInteractHandle handle);

        /** @brief virtual destructor */
        virtual ~Interact(); 

        /** @brief The bitdepth of each component in the openGL frame buffer */
        int getBitDepth(void) const;

        /** @brief Does the openGL frame buffer have an alpha */
        bool hasAlpha(void) const;
        
        /** @brief Returns the size of a real screen pixel under the interact's cannonical projection */
        OfxPointD getPixelScale(void) const;

        /** @brief the background colour */
        OfxRGBColourD getBackgroundColour(void) const;

        /** @brief Set a param that the interact should be redrawn on if its value changes */
        void addParamToSlaveTo(Param *p);

        /** @brief Remova a param that the interact should be redrawn on if its value changes */
        void removeParamToSlaveTo(Param *p);
        
        /** @brief Request a redraw */
        void requestRedraw(void) const;

        /** @brief Swap a buffer in the case of a double bufferred interact, this is possibly a silly one */
        void swapBuffers(void) const;

        ////////////////////////////////////////////////////////////////////////////////
        // override the below in derived classes to do something useful

        /** @brief the function called to draw in the interact */
        virtual bool draw(const DrawArgs &args);

        /** @brief the function called to handle pen motion in the interact
        
        returns true if the interact trapped the action in some sense. This will block the action being passed to 
        any other interact that may share the viewer.
        */
        virtual bool penMotion(const PenArgs &args);

        /** @brief the function called to handle pen down events in the interact 
        
        returns true if the interact trapped the action in some sense. This will block the action being passed to 
        any other interact that may share the viewer.
        */
        virtual bool penDown(const PenArgs &args);

        /** @brief the function called to handle pen up events in the interact 
        
        returns true if the interact trapped the action in some sense. This will block the action being passed to 
        any other interact that may share the viewer.
        */
        virtual bool penUp(const PenArgs &args);

        /** @brief the function called to handle key down events in the interact 
        
        returns true if the interact trapped the action in some sense. This will block the action being passed to 
        any other interact that may share the viewer.
        */
        virtual bool keyDown(const KeyArgs &args);

        /** @brief the function called to handle key up events in the interact 
        
        returns true if the interact trapped the action in some sense. This will block the action being passed to 
        any other interact that may share the viewer.
        */
        virtual bool keyUp(const KeyArgs &args);

        /** @brief the function called to handle key down repeat events in the interact 
        
        returns true if the interact trapped the action in some sense. This will block the action being passed to 
        any other interact that may share the viewer.
        */
        virtual bool keyRepeat(const KeyArgs &args);

        /** @brief Called when the interact is given input focus */
        virtual void gainFocus(const FocusArgs &args);

        /** @brief Called when the interact is loses input focus */
        virtual void loseFocus(const FocusArgs &args);
    };
    
    /** @brief an interact for an image effect overlay */
    class OverlayInteract : public Interact {
    public :
        /** @brief ctor */
        OverlayInteract(OfxInteractHandle handle);

        /** @brief dtor */
        virtual ~OverlayInteract();
    };
};


#undef mDeclareProtectedAssignAndCC

#endif
