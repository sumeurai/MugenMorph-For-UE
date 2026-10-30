# Mugen3D MugenMorph for Unreal Engine

Send a WAV clip to the [Mugen3D](https://mugen3d.ai/) MugenMorph API and play speech-synchronized facial animation on a MetaHuman or any ARKit-named face.

Get keys from the [Developers](https://mugen3d.ai/) page before running the sample.

> Do not commit real keys. Use **Edit → Project Settings → Mugen3D → MugenMorph API** or a local Data Asset.

## Features

- Offline MugenMorph: one WAV in, 61-float ARKit frames + optional audio out
- MetaHuman path: **Mugen3D MugenMorph Face Sync** writes `CTRL_expressions_*` curves **before RigLogic**
- ARKit morph playback (`UMugenMorphComponent`) for custom faces
- Login + protobuf client (`UMugenMorphApiClient`), JSON fallback
- One-click **Setup MugenMorph** on a placed MetaHuman
- Demo level from the official Basic lighting template
- Sample mic record (16 kHz mono WAV → PlayFromAudio)

This public plugin does **not** include Avatar / DNA / TalkStream.

## Requirements

| Item | Note |
| --- | --- |
| Unreal Engine | **5.3** or newer (Fab zip must match your engine version) |
| Engine plugins | **LiveLink** (optional AnimGraph node) and **AudioCapture** (sample mic record). This plugin enables both. |
| Network | HTTPS to `https://api.mugen3d.ai/` |
| Character | Your own MetaHuman (import via Quixel / MetaHuman Creator) or an ARKit blendshape face |
| Audio | PCM WAV. Sample clip is **16 kHz**, 16-bit mono |
| MetaHuman | Enable **RigLogic** in the project (MetaHuman already does this) |

The plugin does **not** ship a MetaHuman mesh, skeleton, or Epic’s `Face_AnimBP`.

## Install

1. Copy the `Mugen3D` folder into your project’s `Plugins/` directory.
2. Restart the editor and enable **Mugen3D** under **Edit → Plugins**.
3. **Edit → Project Settings → Mugen3D → MugenMorph API** and enter Access Key / Secret Key.

Default endpoints:

| Setting | Value |
| --- | --- |
| Base URL | `https://api.mugen3d.ai/` |
| Login | `v1/access/auth` |
| MugenMorph | `v1/mugen-morph/offline-mesh` |

## Quick start

1. **Mugen3D → Create MugenMorph Demo Level** (saves `/Mugen3D/Mugen3D/Sample/MugenMorph/MugenMorph_Demo`, Basic lighting).
2. Fill Access Key / Secret Key (**Mugen3D → API Settings**).
3. Import your MetaHuman (or ARKit face) into the level.
4. Select the character → **Mugen3D → Setup MugenMorph on Selected Actor**.
5. Play. Overlay: **Browse** / **Start Record** / **Stop Record** / **Interrupt**. Keys: `J` / `R` / `I`.

Empty keys show a hint and do not crash. Press **J** to pick a local WAV. **R** (or Start/Stop Record) captures the default microphone as 16 kHz mono WAV and sends it through `PlayFromAudio`.

### What Setup MugenMorph does

- Adds `UMugenMorphComponent` (keeps the MetaHuman Face AnimBP; does not compile the character Blueprint)
- Builds or reuses `Mugen3D_MetaHuman_FaceAnim` (Face Sync **before RigLogic**)
- Assigns that AnimBP to the Face mesh
- Wires the demo sample actor’s `FaceMesh` when one is in the level

Open the Face AnimBP and confirm:

`… → Mugen3D MugenMorph Face Sync → RigLogic → Output Pose`

## Blueprint / C++

```cpp
MugenMorphComponent->Init(0, MotionType::ARKit, FaceMesh); // keeps AnimBP unless you pass true
ApiClient->PlayFromAudio(WavBytes);
```

1. `UMugenMorphComponent::Init` — leave `bReplaceAnimInstance` false for MetaHuman.
2. `UMugenMorphApiClient::PlayFromAudio` — logs in automatically when the token is empty.
3. Bind `AMugenMorphManager`: On Start Speech / On Stop Speech / On Stop Motion.
4. `UMugenMorphComponent::Interrupt()` or `UMugenMorphApiClient::Interrupt()` to cancel.

## API

1. **Login** `POST v1/access/auth`  
   Body `{ "accessKey", "secretKey" }` → `accessToken`  
   Later requests: `Authorization: <token>` (no `Bearer` prefix).
2. **MugenMorph** `POST v1/mugen-morph/offline-mesh`  
   Headers: `Authorization`, `Accept: application/x-protobuf`  
   Body `{ "traceId", "data": "<wav-base64>" }`

| Response field | Meaning |
| --- | --- |
| `fps` | Blendshape frame rate (30 if missing) |
| `audio` | Optional WAV / MP3 / PCM. If empty, the local clip is played |
| `blendshapes` | Little-endian float32, **61 values per frame** (52 ARKit + head / eye extras) |

Legacy `v1/audio-to-face/mesh` JSON still works if you set that path (no `offline`).

## Character notes

- Official MetaHuman mouths are **RigLogic curves**, not `SetMorphTarget("JawOpen")`.
- Setup maps ARKit names to `CTRL_expressions_*` (for example `JawOpen` → `CTRL_expressions_jawOpen`).
- The Face AnimBP Target Skeleton is the project’s `/Game/MetaHumans/Common/Face/Face_Archetype_Skeleton` (same pattern as Convai). Import a MetaHuman first so that skeleton exists.
- Custom faces: name morphs close to ARKit (`JawOpen`, `MouthSmileLeft`, …).

More sample notes: [`Content/Mugen3D/Sample/MugenMorph/README_MugenMorph.txt`](Content/Mugen3D/Sample/MugenMorph/README_MugenMorph.txt).

## License

Distributed on Fab under the Fab Standard License.  
Mugen3D cloud API: your account terms on [mugen3d.ai](https://mugen3d.ai/).  
Third-party libraries: [ThirdPartyNotices.md](ThirdPartyNotices.md).
