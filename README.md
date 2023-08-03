## Melatonin Component Inspector


A JUCE module that gives you the ability to inspect and visually edit (non-destructively) components in your UI.

It's inspired by [Figma](https://figma.com) (where I prefer to design UI), browser web inspectors and Jim Credland's [Component Debugger](https://github.com/jcredland/juce-toys/blob/master/jcf_debug/source/component_debugger.cpp) [juce-toys](https://github.com/jcredland/juce-toys). 

A big hearty thanks to [Dmytro Kiro](https://github.com/dikadk) for many of the current features.

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213710642-389d4313-5b5a-4656-859e-ea014c859021.jpg" width="500"/>
</p>


<p align="center">
✨<br/>
✨✨<br/>
✨✨✨<br/>
<b><i>...the features...</i></b><br/>
✨✨✨<br/>
✨✨<br/>
✨<br/>
</p>

## Browse and select components visually

Point n' click to inspect a component, see its size and distance to parent.

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213708404-90948d45-6440-455d-bac0-662acf5a1d70.gif" width="600"/>
</p>


## Explore the component heirarchy and see what's visible

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213709528-b0f4d3dc-61bb-4e72-aae5-7c414ee5e798.gif" width="400"/>
</p>


## Preview Component

See what exactly is drawing on a per-component basis, even if it's hidden.


<p align="center">
<img src="https://user-images.githubusercontent.com/472/213702194-c3c586e1-981f-4760-b095-5d7a13f322b7.gif" width="600"/>
</p>

## Filter and find components by name

Names are derived from stock components, label/button text, or demangled class names.


<p align="center">
<img src="https://user-images.githubusercontent.com/472/213702600-2e479677-8a6e-459b-ba38-1db93be689e3.gif" width="400"/>
</p>


## Edit component position and spacing

There's like...4 different ways to do this, visually and numerically...

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213701508-eaf1dfc6-4465-4581-915e-7d788d47e08c.gif" width="600"/>
</p>

## View and edit a components *padding*

This requires that padding is stored as component properties (e.g. `paddingLeft`, `paddingTop`, etc), see my `PaddedComponent` [base class as an example](https://gist.github.com/sudara/eed6b8bb3b960b4c2156a0883913ea15).

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213940575-0f9fa289-6b00-4899-b07e-9c9ae9e500c9.gif" width="600"/>
</p>


## Inspect component properties

See the most important component properties at a glance, including look and feels, fonts for labels, etc.

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213707219-dc8619c1-8800-4267-b3f3-0c9911817a63.jpg" width="600"/>
</p>


## Nudge components around 

Verify new values, get things pixel perfect.

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213701074-a48414e8-a86c-4881-a911-392710078ce7.gif" width="600"/>
</p>

### View spacing relative to siblings/neighbors

Hold "alt" while component is selected. A Figma inspired feature.

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213705531-d9887c7b-15a7-4b78-bab4-6868765d45a5.gif" width="450"/>
</p>

### Basic color picker

Displays simple RGB values.

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213706422-42c5eb90-c41f-45f8-9a0c-0125b125a622.gif" width="600"/>
</p>


## How to install


### CMake option #1: `FetchContent`

Example usage:
```cmake
include (FetchContent)

FetchContent_Declare (melatonin_inspector
  GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
  GIT_TAG origin/main)

FetchContent_MakeAvailable (melatonin_inspector)

target_link_libraries (yourTarget PRIVATE Melatonin::Inspector)
```

### CMake option #2 git submodules and `add_subdirectory`

If you are a git submodule aficionado, add this repository as a git submodule to your project:
```sh
git submodule add -b main https://github.com/sudara/melatonin_inspector.git modules/melatonin_inspector
```
and then simply call `add_subdirectory` in your CMakeLists.txt:
```cmake
add_subdirectory (modules/melatonin_inspector)
```

To update melatonin_inspector down the road (gasp! maintained dependencies!?) you can:
```git
git submodule update --remote --merge modules/melatonin_inspector
```

### Tell JUCE about the module

After your `juce_add_plugin` call, you'll need to link the module to your plugin, for example:

```
target_link_libraries("YourProject" PRIVATE melatonin_inspector)
```

Note: you don't have to call `juce_add_module`, that's handled by CMake automatically.


If you use projucer, add the module manually.
 
## 3. Add an include to your Plugin Editor

Include the module header:

```cpp
#include "melatonin_inspector/melatonin_inspector.h"
```

## 4. Add the inspector as a private member to your Editor

The easiest way to get started is to pass a reference to the root component of your UI (typically the Editor itself like in this example, but you could also inspect anything that derives from `juce::Component`).

```cpp
melatonin::Inspector inspector { *this };
```

If you prefer the inspector open in the disabled state by default, you can pass false as the second argument.

```cpp
melatonin::Inspector inspector { *this, false };
```

## 5. Set it visible

When the inspector as an editor member, you can use `cmd/ctrl i` to toggle whether the inspector is enabled.

`setVisible` on the member will also pop the window open.

What I do is have a GUI toggle that pops open the window and enables inspection:

```cpp
// open the inspector window
inspector.setVisible(true); 

// enable the inspector
inspector.toggle(true);
```

## 6. Optional: Make it smarter 

Setting up as above means that the inspector will always be constructed with your editor. Clicking close on the inspector's `DocumentWindow` will just hide it while disabling inspection. 

If you wrap the inspector with `#if DEBUG` this might be fine for you.

However, if you'd plan to ship a product that includes the inspector, or otherwise want to lazily construct it to be more efficient, use a `unique_ptr` instead and set the `onClose` callback to reset the pointer.

```c++
// PluginEditor.h
std::unique_ptr<melatonin::Inspector> inspector;

// in some button on-click logic
// replace `this` with a reference to your editor if necessary
if (!inspector)
{
    inspector = std::make_unique<melatonin::Inspector> (*this);
    inspector->onClose = [this]() { inspector.reset(); };
}

inspector->setVisible (true);
```
Thanks to @FigBug for this feature.

## FAQ

### Can I save my component resizes or edits?

Nope! 

For that, one would need a component system relying on data for placement and size vs. code. See [Daniel's Foley GUI Magic](https://github.com/ffAudio/foleys_gui_magic).

### How is the component hierarchy created?

It traverses components from the root, building a `TreeView`. 

In the special case of `TabbedComponent`, each tab is added as a child. 
