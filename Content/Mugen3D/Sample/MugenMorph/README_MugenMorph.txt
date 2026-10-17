Mugen3D MugenMorph (GitHub / Fab)
==================================

Public slice: login + offline Audio-to-Face. Get Access Key / Secret Key from https://mugen3d.ai/
Recommended face path: UMugenMorphComponent + Face AnimBP node "Mugen3D MugenMorph Face Sync"
(writes ARKit / CTRL_expressions_* curves before RigLogic).
Optional: Mugen3DIK LiveLink node.

Not in this package
- TalkStream /talk protobuf streaming
- Avatar / DNA module (Source/Mugen3D)

Setup
-----
1. Enable plugin Mugen3D (or copy Plugins/Mugen3D into your project).
2. Project Settings -> Mugen3D -> MugenMorph API: fill AccessKey / SecretKey from
   https://mugen3d.ai/ Developers
   Defaults: BaseUrl=https://api.mugen3d.ai/
             LoginPath=v1/access/auth
             MugenMorphMeshPath=v1/audio-to-face/offline-mesh
   Or create a Data Asset: Miscellaneous -> Data Asset -> MugenMorph API Settings
   (client override > data asset > project settings).
3. Place AMugenMorphManager (or a Blueprint subclass) in the level if you need
   Blueprint events. Otherwise USumeruMugenMorphSubsystem::EnsureManager() spawns one.
4. Import your MetaHuman or ARKit face (not included in this plugin).
   Select it and run Mugen3D -> Setup MugenMorph on Selected Actor.
   Or add UMugenMorphComponent yourself:
   - FaceCom can stay empty; BeginPlay finds the Face mesh
   - Leave bReplaceAnimInstance off so the MetaHuman Face AnimBP stays
   - Setup inserts "Mugen3D MugenMorph Face Sync" before RigLogic; save Face_AnimBP
   - MotionType ARKit (JawOpen / CTRL_expressions_jawOpen)

Online API
----------
Login: POST v1/access/auth  {accessKey, secretKey}
       Header Authorization is the raw accessToken (no Bearer prefix).

MugenMorph (default): POST v1/audio-to-face/offline-mesh
  Accept: application/x-protobuf
  Body: { "traceId", "data": "<wav-base64>" }
  Response: protobuf {code, message, data{fps, num_frames, audio, blendshapes}}
            or JSON fallback {code, data.emoteKey, data.audioKey, fps}

Legacy fallback: set MugenMorphMeshPath to v1/audio-to-face/mesh (no "offline")
  Body: { status, dialogueBase64, lastDialogueBase64, traceId }

Sample
------
Menu: Mugen3D -> Create MugenMorph Demo Level  (saves /Mugen3D/Mugen3D/Sample/MugenMorph/MugenMorph_Demo)
      Mugen3D -> API Settings

1. Fill Access Key / Secret Key (empty keys: overlay warns, PIE does not crash).
2. Import a MetaHuman, select it, Mugen3D -> Setup MugenMorph on Selected Actor.
   Setup wires the sample actor FaceMesh when one is in the level.
3. PIE overlay: Browse / Start Record / Stop Record / Interrupt.
   Keys: J / R / I (pick a WAV, or record then PlayFromAudio).
   Record uses the default microphone, encodes 16 kHz mono PCM WAV, then PlayFromAudio.

The client logs in if needed, POSTs the wav, then feeds PCM + 61-float frames
into AMugenMorphManager via EnqueueExternalSpeech.
Response audio may be WAV, PCM16, or MP3. If the server omits audio, the local wav is used.
A second PlayFromAudio while busy is ignored.
UMugenMorphApiClient.Interrupt() stops HTTP, audio, and mouth.

Blueprint
---------
1. UMugenMorphComponent.Init(FaceId, ARKit, FaceMesh)  (AnimBP is kept by default)
2. UMugenMorphApiClient.Login()  (optional; PlayFromAudio logs in automatically)
3. UMugenMorphApiClient.PlayFromAudio(WavBytes)
   Sample actor also has StartRecord / StopRecord (mic → 16 kHz WAV → PlayFromAudio)
4. Bind AMugenMorphManager: On Start Speech / On Stop Speech / On Stop Motion
5. UMugenMorphComponent.Interrupt() to cancel playback

Lip sync
- bSyncEmoteToAudioClock maps mouth progress to queued PCM duration.
- Fine-tune Manager.LipSyncLeadSeconds (default 0.02).

AnimGraph
- Setup creates /Game/Mugen3D/MetaHumans/Animations/Mugen3D_MetaHuman_FaceAnim
  (copy of your Face AnimBP + Mugen3D MugenMorph Face Sync before RigLogic) and assigns it to Face.
- Skeleton is the project's Face_Archetype_Skeleton, same pattern as Convai.
- You can also add the node from the AnimGraph palette: Audio To Face.
- "Mugen3D MugenMorph Pose" (LiveLink) is a separate optional node.
