## Melatonin Component Inspector

This is a JUCE module that gives you the ability to inspect and resize components in your UI.

It's inspired by [Figma](https://figma.com) (where I prefer to design UI), browser web inspectors and Jim Credland's [Component Debugger](https://github.com/jcredland/juce-toys/blob/master/jcf_debug/source/component_debugger.cpp) [juce-toys](https://github.com/jcredland/juce-toys). 

https://user-images.githubusercontent.com/472/115873623-6f2e4700-a443-11eb-9ba1-03d992cc12e1.mp4

## How to install

You have several options. In all cases, the exported target you should link against is: `Melatonin::Inspector`.

### Option #1: `FetchContent`

Example usage:
```cmake
include (FetchContent)

FetchContent_Declare (melatonin_inspector
  GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
  GIT_TAG origin/main)

FetchContent_MakeAvailable (melatonin_inspector)

target_link_libraries (yourTarget PRIVATE Melatonin::Inspector)
```

### Option #2 git submodules and `add_subdirectory`

If you are a git submodule aficionado, add this repository as a git submodule to your project:
```sh
git submodule add -b main https://github.com/sudara/melatonin_inspector.git modules/melatonin_inspector
```
and then simply call `add_subdirectory` in your CMakeLists.txt:
```cmake
add_subdirectory (modules/melatonin_inspector)

target_link_libraries (yourTarget PRIVATE Melatonin::Inspector)
```

To update melatonin_inspector down the road (gasp! maintained dependencies!?) you can:
```git
git submodule update --remote --merge modules/melatonin_inspector
```


 
## 3. Add an include to your Plugin Editor

Include the module header:

```cpp
#include "melatonin_inspector/melatonin_inspector.h"
```

## 4. Add the inspector as a private member to your Editor

Pass a reference to the root component of your UI (typically the  Editor itself, but you could also inspect another window, etc).

```cpp
melatonin::Inspector inspector { *this };
```

If you'd like the inspector to be disabled by default, pass false as the second argument.

```cpp
melatonin::Inspector inspector { *this, false };
```

This is what I do. I have a GUI toggle to enable it when necessary which calls 

```cpp
inspector.setVisible(true); 
inspector.toggle(true);
```

## 5. Set it visible

Speaking of, out of the box, the additional window won't popup until you call `inspector.setVisible(true)` somewhere on the inspector!

## Usage & Features

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
