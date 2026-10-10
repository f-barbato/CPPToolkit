# 0.1.0-b004

### Fixed

- Explicitly prohibit widget copy and move operations to preserve exclusive child ownership and stable identity, fixing MSVC C2280 errors in Windows shared builds.
