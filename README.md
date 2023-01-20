# Melatonin Inspector


A JUCE module that gives you the ability to inspect and visually edit (non-destructively) components in your UI.

It's inspired by [Figma](https://figma.com) (where I prefer to design UI), browser web inspectors and Jim Credland's [Component Debugger](https://github.com/jcredland/juce-toys/blob/master/jcf_debug/source/component_debugger.cpp) [juce-toys](https://github.com/jcredland/juce-toys). 

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213710642-389d4313-5b5a-4656-859e-ea014c859021.jpg" width="500"/>
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

Thanks Dmytro!

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213702194-c3c586e1-981f-4760-b095-5d7a13f322b7.gif" width="600"/>
</p>

## Filter and find components by name

We grab names from stock components, label/button text, otherwise we demangle the class names.

Thanks Dmytro!

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213702600-2e479677-8a6e-459b-ba38-1db93be689e3.gif" width="400"/>
</p>


## Edit component position and spacing

There's like...4 different ways to do this, visually and numerically...

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213701508-eaf1dfc6-4465-4581-915e-7d788d47e08c.gif" width="600"/>
</p>

## Inspect component properties

See the most important component properties at a glance, including look and feels, fonts for labels, etc.

Thanks Dmytro!

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213707219-dc8619c1-8800-4267-b3f3-0c9911817a63.jpg" width="600"/>
</p>


## Nudge components around 

Good for (non-destructively) verifying what new positions should be, getting things pixel perfect.

Thanks Dmytro!

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213701074-a48414e8-a86c-4881-a911-392710078ce7.gif" width="600"/>
</p>

### Hold "alt" while component is selected to view positioning relative to neighbors

Figma inspired feature.

Thanks Dmytro!

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213705531-d9887c7b-15a7-4b78-bab4-6868765d45a5.gif" width="450"/>
</p>

### Basic color picker

Displays simple RGB values.

Thanks Dmytro!

<p align="center">
<img src="https://user-images.githubusercontent.com/472/213706422-42c5eb90-c41f-45f8-9a0c-0125b125a622.gif" width="600"/>
</p>


## How to install

### 1. Download

Assuming 

1. you use git and 
2. you have modules in a `modules` subfolder of your project

this will set you up with a git submodule tracking the `main` branch:

```git
git submodule add -b main https://github.com/sudara/melatonin_inspector modules/melatonin_inspector
git commit -m "Added melatonin_inspector submodule."
```

To update melatonin_inspector down the road (gasp! maintained dependencies!?) you can:
```git
git submodule update --remote --merge modules/melatonin_inspector
```

### 2. Tell JUCE about the module

If you use CMake, inform JUCE about the module in your `CMakeLists.txt`. Important: Do this *before* your call to `juce_add_module`!!
```
juce_add_module("modules/melatonin_inspector")
```

In addition, you'll need to link the module to your plugin, for example:

```
target_link_libraries("YourProject" PRIVATE melatonin_inspector)
```

If you use projucer, add the module manually.
 
### 3. Add an include to your Editor

Include the module header:

```cpp
#include "melatonin_inspector/melatonin_inspector.h"
```

### 4. Add the inspector as a private member to your Editor

Pass a reference to the root component of your UI (typically the  Editor itself, but you could also inspect another window, etc).

```cpp
melatonin::Inspector inspector { *this };
```

If you'd like the inspector to be disabled by default, pass false as the second argument.

```cpp
melatonin::Inspector inspector { *this, false };
```

### 5. Set it visible

Out of the box, the window won't popup until you call `setVisible(true)` on the inspector!

## FAQ

### Can I save my resizes or edits?

Nope! For that, one would need a component system relying on data for placement and size vs. code. See [Daniel's Foley GUI Magic](https://github.com/ffAudio/foleys_gui_magic).

### How is the component hierarchy created?

It traverses components from the root, building a `TreeView`. In the special case of `TabbedComponent`, each tab is added as a child. 
