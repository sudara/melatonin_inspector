/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace InspectorBinaryData
{
    extern const char*   _DS_Store;
    const int            _DS_StoreSize = 6148;

    extern const char*   Clear_png;
    const int            Clear_pngSize = 1419;

    extern const char*   Eyedropperoff_png;
    const int            Eyedropperoff_pngSize = 673;

    extern const char*   Eyedropperon_png;
    const int            Eyedropperon_pngSize = 671;

    extern const char*   Logo_png;
    const int            Logo_pngSize = 25781;

    extern const char*   Search_png;
    const int            Search_pngSize = 941;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 6;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
