## Melatonin Component Inspector

This is a JUCE module that helps you lay out your JUCE UI by giving you the ability to inspect and resize components. 

It's inspired by [Figma](https://figma.com) (where I prefer to design UI), browser web inspectors and Jim Credland's [Component Debugger](https://github.com/jcredland/juce-toys/blob/master/jcf_debug/source/component_debugger.cpp) [juce-toys](https://github.com/jcredland/juce-toys). 

## Features

### Hover over Component to view size and bounds to parent


![](https://user-images.githubusercontent.com/472/115127488-20ae2200-9fd7-11eb-9f68-d5a7bfa6ce40.mp4)


### Single click a Component to select it and resize

(Note: size changes aren't permanent, it's similar to how you can play with things in a browser's web inspector)

### Hold "alt" after a Component is selected to view positioning relative to neighbors

(Coming soon)


### Navigate and select from the Component tree

The names used in the UI are the Component name, if specified (e.g. JUCE buttons), otherwise demangled class names

## How to install

### Add this as a JUCE module

Assuming you use git and you have modules in a `modules` subfolder of your project, this will set you up with a git submodule tracking the `main` branch:

```git
git submodule add -b main https://github.com/sudara/melatonin_inspector modules/melatonin_inspector
git commit -m "Added melatonin_inspector submodule."
```

When you want to update melatonin_inspector, you can now run
```git
git submodule update --remote --merge modules/melatonin_inspector
```

If you use CMake, you can inform JUCE about the module in your `CMakeLists.txt`:
```
juce_add_module("modules/melatonin_inspector")
```
 
### Add as a member of your Editor, etc

Make sure to include the module header:

```cpp
#include "melatonin_inspector/melatonin_inspector.h"
```

Add the inspector as a private member to your Editor, passing a reference to the root component of your UI (typically the the Editor itself, but you could also inspect another window, etc).

```cpp
MelatoninInspector inspector { *this };
```

## FAQ

### Can I save my resizes or edits?

Nope! For that, one would need a component system relying on data for placement and size vs. code. See [Daniel's Foley GUI Magic](https://github.com/ffAudio/foleys_gui_magic).

### How is the component hierarchy created?

It traverses components from the root, building a `TreeView`. In the special case of `TabbedComponent, each tab is added. 
