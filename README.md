# SumeruAI Audio-to-Face for Unreal Engine

Send a WAV clip to the [SumeruAI](https://www.sumeruai.us/) Audio-to-Face API and play speech-synchronized facial animation on a MetaHuman or any ARKit-named face.

This is the Unreal counterpart of the Unity SumeruAI AudioToFace plugin: **the same Access Key, Secret Key, login, and offline-mesh API**.

Get keys from the [Developers](https://www.sumeruai.us/) page before running the sample.

> Do not commit real keys. Use **Edit → Project Settings → SumeruAI → ATF API** or a local Data Asset.

## Features

- Offline Audio-to-Face: one WAV in, 61-float ARKit frames + optional audio out
- MetaHuman path: **SumeruAI ATF Face Sync** writes `CTRL_expressions_*` curves **before RigLogic**
- ARKit morph playback (`UATFComponent`) for custom faces
- Login + protobuf client (`UATFApiClient`), JSON fallback
- One-click **Setup ATF** on a placed MetaHuman
- Demo level from the official Basic lighting template

This public plugin does **not** include Avatar / DNA / TalkStream.

## Requirements

| Item | Note |
| --- | --- |
| Unreal Engine | **5.3** or newer |
| Network | HTTPS to `https://api.sumeruai.us/` |
| Character | Your own MetaHuman (import via Quixel / MetaHuman Creator) or an ARKit blendshape face |
| Audio | PCM WAV. Sample clip is **16 kHz**, 16-bit mono |
| MetaHuman | Enable **RigLogic** in the project (MetaHuman already does this) |
| Optional | LiveLink — only if you use the extra ATF AnimGraph node |

The plugin does **not** ship a MetaHuman mesh, skeleton, or Epic’s `Face_AnimBP`.

## Install

1. Copy the `SumeruAI` folder into your project’s `Plugins/` directory.
2. Restart the editor and enable **SumeruAI** under **Edit → Plugins**.
3. **Edit → Project Settings → SumeruAI → ATF API** and enter Access Key / Secret Key.

Default endpoints:

| Setting | Value |
| --- | --- |
| Base URL | `https://api.sumeruai.us/` |
| Login | `v1/access/auth` |
| Audio-to-Face | `v1/audio-to-face/offline-mesh` |

## Quick start

1. **SumeruAI → Create ATF Demo Level** (saves `/SumeruAI/Sample/AudioToFace/ATF_Demo`, Basic lighting).
2. Fill Access Key / Secret Key (**SumeruAI → API Settings**).
3. Import your MetaHuman (or ARKit face) into the level.
4. Select the character → **SumeruAI → Setup ATF on Selected Actor**.
5. Play. Overlay: **Play WAV** / **Browse** / **Interrupt**. Keys: `P` / `J` / `I`.

Empty keys show a hint and do not crash. Press **J** to pick any local WAV if you do not want the bundled silence clip.

### What Setup ATF does

- Adds `UATFComponent` (keeps the MetaHuman Face AnimBP; does not compile the character Blueprint)
- Builds or reuses `SumeruAI_MetaHuman_FaceAnim` (Face Sync **before RigLogic**)
- Assigns that AnimBP to the Face mesh
- Wires the demo sample actor’s `FaceMesh` when one is in the level

Open the Face AnimBP and confirm:

`… → SumeruAI ATF Face Sync → RigLogic → Output Pose`

## Blueprint / C++

```cpp
AtfComponent->Init(0, MotionType::ARKit, FaceMesh); // keeps AnimBP unless you pass true
ApiClient->PlayFromAudio(WavBytes);
```

1. `UATFComponent::Init` — leave `bReplaceAnimInstance` false for MetaHuman.
2. `UATFApiClient::PlayFromAudio` — logs in automatically when the token is empty.
3. Bind `AAudioToFaceManager`: On Start Speech / On Stop Speech / On Stop Motion.
4. `UATFComponent::Interrupt()` or `UATFApiClient::Interrupt()` to cancel.

## API (same as Unity)

1. **Login** `POST v1/access/auth`  
   Body `{ "accessKey", "secretKey" }` → `accessToken`  
   Later requests: `Authorization: <token>` (no `Bearer` prefix).
2. **Audio-to-Face** `POST v1/audio-to-face/offline-mesh`  
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

More sample notes: [`Content/Sample/AudioToFace/README_ATF.txt`](Content/Sample/AudioToFace/README_ATF.txt).

## License

Plugin source: see [LICENSE](LICENSE).  
SumeruAI cloud API: your account terms on [sumeruai.us](https://www.sumeruai.us/).  
Third-party libraries: [ThirdPartyNotices.md](ThirdPartyNotices.md).
