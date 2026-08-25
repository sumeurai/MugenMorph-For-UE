Sumeru ATF (GitHub / Fab)
=========================

Public slice: login + offline Audio-to-Face. Same Access Key / Secret Key as the Unity plugin.
Recommended face path: UATFComponent + Face AnimBP node "SumeruAI ATF Face Sync"
(writes ARKit / CTRL_expressions_* curves before RigLogic).
Optional: SumeruIK LiveLink node.

Not in this package
- TalkStream /talk protobuf streaming
- Avatar / DNA module (Source/SumeruAI)

Setup
-----
1. Enable plugin SumeruAI (or copy Plugins/SumeruAI into your project).
2. Project Settings -> SumeruAI -> ATF API: fill AccessKey / SecretKey from
   https://www.sumeruai.us/ Developers
   Defaults: BaseUrl=https://api.sumeruai.us/
             LoginPath=v1/access/auth
             ATFMeshPath=v1/audio-to-face/offline-mesh
   Or create a Data Asset: Miscellaneous -> Data Asset -> Sumeru ATF API Settings
   (client override > data asset > project settings).
3. Place AAudioToFaceManager (or a Blueprint subclass) in the level if you need
   Blueprint events. Otherwise USumeruATFSubsystem::EnsureManager() spawns one.
4. Import your MetaHuman or ARKit face (not included in this plugin).
   Select it and run SumeruAI -> Setup ATF on Selected Actor.
   Or add UATFComponent yourself:
   - FaceCom can stay empty; BeginPlay finds the Face mesh
   - Leave bReplaceAnimInstance off so the MetaHuman Face AnimBP stays
   - Setup inserts "SumeruAI ATF Face Sync" before RigLogic; save Face_AnimBP
   - MotionType ARKit (JawOpen / CTRL_expressions_jawOpen)

Online API (Unity-compatible)
-----------------------------
Login: POST v1/access/auth  {accessKey, secretKey}
       Header Authorization is the raw accessToken (no Bearer prefix).

ATF (default): POST v1/audio-to-face/offline-mesh
  Accept: application/x-protobuf
  Body: { "traceId", "data": "<wav-base64>" }
  Response: protobuf {code, message, data{fps, num_frames, audio, blendshapes}}
            or JSON fallback {code, data.emoteKey, data.audioKey, fps}

Legacy fallback: set ATFMeshPath to v1/audio-to-face/mesh (no "offline")
  Body: { status, dialogueBase64, lastDialogueBase64, traceId }

Sample
------
Menu: SumeruAI -> Create ATF Demo Level  (saves ATF_Demo)
      SumeruAI -> API Settings

1. Fill Access Key / Secret Key (empty keys: overlay warns, PIE does not crash).
2. Import a MetaHuman, select it, SumeruAI -> Setup ATF on Selected Actor.
   Setup wires the sample actor FaceMesh when one is in the level.
3. PIE overlay: Play WAV / Browse / Interrupt. Keys: P / J / I.
   Placeholder wav: Content/Sample/AudioToFace/Audio/silence_16k.wav

The client logs in if needed, POSTs the wav, then feeds PCM + 61-float frames
into AAudioToFaceManager via EnqueueExternalSpeech.
Response audio may be WAV, PCM16, or MP3. If the server omits audio, the local wav is used.
A second PlayFromAudio while busy is ignored (Unity atfRequesting).
UATFApiClient.Interrupt() stops HTTP, audio, and mouth.

Blueprint (Unity PlayFromAudio equivalent)
-----------------------------------------
1. UATFComponent.Init(FaceId, ARKit, FaceMesh)  (AnimBP is kept by default)
2. UATFApiClient.Login()  (optional; PlayFromAudio logs in automatically)
3. UATFApiClient.PlayFromAudio(WavBytes)
4. Bind AAudioToFaceManager: On Start Speech / On Stop Speech / On Stop Motion
5. UATFComponent.Interrupt() to cancel playback

Lip sync
- bSyncEmoteToAudioClock maps mouth progress to queued PCM duration.
- Fine-tune Manager.LipSyncLeadSeconds (default 0.02).

AnimGraph
- Setup creates /Game/SumeruAI/MetaHumans/Animations/SumeruAI_MetaHuman_FaceAnim
  (copy of your Face AnimBP + SumeruAI ATF Face Sync before RigLogic) and assigns it to Face.
- Skeleton is the project's Face_Archetype_Skeleton, same pattern as Convai.
- You can also add the node from the AnimGraph palette: Audio To Face.
- "Sumeru AI Audio To Face" (LiveLink) is a separate optional node.
