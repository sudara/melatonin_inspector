## Melatonin Component Inspector

This is a JUCE module that gives you the ability to inspect and resize components in your UI.

It's inspired by [Figma](https://figma.com) (where I prefer to design UI), browser web inspectors and Jim Credland's [Component Debugger](https://github.com/jcredland/juce-toys/blob/master/jcf_debug/source/component_debugger.cpp) [juce-toys](https://github.com/jcredland/juce-toys). 

https://user-images.githubusercontent.com/472/115873623-6f2e4700-a443-11eb-9ba1-03d992cc12e1.mp4

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
