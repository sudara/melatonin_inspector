## Melatonin Component Inspector

This is a JUCE module that gives you the ability to inspect and resize components in your UI.

It's inspired by [Figma](https://figma.com) (where I prefer to design UI), browser web inspectors and Jim Credland's [Component Debugger](https://github.com/jcredland/juce-toys/blob/master/jcf_debug/source/component_debugger.cpp) [juce-toys](https://github.com/jcredland/juce-toys). 

![](https://user-images.githubusercontent.com/472/115612633-c7eacc00-a2eb-11eb-8333-1b43ff7ed793.mp4)

## How to install

### 1. Add the module

Assuming 

1. you use git and 
2. you have modules in a `modules` subfolder of your project

this will set you up with a git submodule tracking the `main` branch:

```git
git submodule add -b main https://github.com/sudara/melatonin_inspector modules/melatonin_inspector
git commit -m "Added melatonin_inspector submodule."
```

To update melatonin_inspector, you can:
```git
git submodule update --remote --merge modules/melatonin_inspector
```

If you use CMake, you can inform JUCE about the module in your `CMakeLists.txt`:
```
juce_add_module("modules/melatonin_inspector")
```

If you use projucer, you will have to add the module manually.
 
### 2. Add as a member of your Editor

Make sure to include the module header:

```cpp
#include "melatonin_inspector/melatonin_inspector.h"
```

Add the inspector as a private member to your Editor, passing a reference to the root component of your UI (typically the the Editor itself, but you could also inspect another window, etc).

```cpp
MelatoninInspector inspector { *this };
```

If you'd like the inspector to be disabled by default, pass false as the second argument.

```cpp
MelatoninInspector inspector { *this, false };
```

## Features

### Hover over Component to view size and bounds to parent

### Single click a Component to select it and resize

(Note: size changes aren't permanent, it's similar to how you can play with things in a browser's web inspector)

### Hold "alt" after a Component is selected to view positioning relative to neighbors

(Coming soon)

### Navigate and select from the Component tree

The names used in the UI are the Component name, if specified (e.g. JUCE buttons), otherwise demangled class names

## FAQ

### Can I save my resizes or edits?

Nope! For that, one would need a component system relying on data for placement and size vs. code. See [Daniel's Foley GUI Magic](https://github.com/ffAudio/foleys_gui_magic).

### How is the component hierarchy created?

It traverses components from the root, building a `TreeView`. In the special case of `TabbedComponent`, each tab is added as a child. 
