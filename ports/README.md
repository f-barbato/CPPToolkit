# CPPToolkit overlay ports

Consumers must use this entire `ports/` directory as their overlay, not just
`ports/cpptoolkit`. The ImGui and ImPlot registry ports enforce static-only
linkage. These overlays retain their upstream features while enabling static
or shared builds, PIC and generated Windows export/import headers.

Shared UI builds need a single ImGui/ImPlot implementation across the application,
the bridge and CPPToolkit. Mixing separate static copies across DLL boundaries
would duplicate context state. Use a consistent dynamic triplet and these ports
instead of relying on incidental ELF symbol interposition.

`imgui/` (1.92.9, upstream tag 1.92.9b) and `implot/` (1.0) are derived from
Microsoft vcpkg commit `5dd2e1600d049b498ff9fb9fe15997533ae0c804`.
The vcpkg port build files are MIT licensed; see `LICENSE.vcpkg`.
The library sources are downloaded at pinned revisions and retain their
upstream licenses. Local changes remove the static-only restriction, add PIC
and generated export headers, and make ImPlot's ImGui dependency transitive.
Update these overlays deliberately when upgrading the dependency versions.

The CPPToolkit port currently builds the repository containing this directory.
Keep that checkout available while building consumers. Root manifest overrides
are not inherited by consumer manifests; consumers can choose their own baseline
and compatible raylib version (at least 5.5).
