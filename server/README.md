# VAir Server (Phase 3)

**Status**: 📋 Planned - Not yet implemented

This directory will contain the server-side components for Phase 3 of the VAir project: a self-hosted voice processing server with Vietnamese speech support.

## Overview

Phase 3 extends VAir with a self-hosted server for:
- **Self-hosted xiaozhi-esp32-server** deployment
- **Vietnamese speech-to-text (STT)** for voice command recognition
- **Vietnamese text-to-speech (TTS)** for voice responses
- **Wake word detection** in Vietnamese (future goal)
- **Local processing** for privacy and reduced latency

## Architecture

### Server Components

```
┌─────────────────────────────────────────┐
│     VAir Self-Hosted Server             │
├─────────────────────────────────────────┤
│                                         │
│  ┌─────────────────────────────────┐   │
│  │ xiaozhi-esp32-server            │   │
│  │ (xinnan-tech/xiaozhi-esp32-     │   │
│  │  server)                        │   │
│  └─────────────────────────────────┘   │
│               ↕                         │
│  ┌─────────────────────────────────┐   │
│  │ Vietnamese STT Model            │   │
│  │ (Whisper / VAIS / Other)        │   │
│  └─────────────────────────────────┘   │
│               ↕                         │
│  ┌─────────────────────────────────┐   │
│  │ Vietnamese TTS Model            │   │
│  │ (VITS / Edge-TTS / Other)       │   │
│  └─────────────────────────────────┘   │
│               ↕                         │
│  ┌─────────────────────────────────┐   │
│  │ (Optional) Vietnamese Wake      │   │
│  │ Word Detection                  │   │
│  └─────────────────────────────────┘   │
│                                         │
└─────────────────────────────────────────┘
              ↕
       Network (Local WiFi)
              ↕
┌─────────────────────────────────────────┐
│     ESP32-S3 Voice Device (Phase 2)     │
│     - Microphone                        │
│     - Speaker                           │
│     - IR LED                            │
└─────────────────────────────────────────┘
```

## Server Requirements

### Hardware
- **CPU**: Multi-core processor (4+ cores recommended)
- **RAM**: 8GB minimum, 16GB recommended (for STT/TTS models)
- **Storage**: 20GB+ SSD for models and runtime data
- **Network**: Gigabit Ethernet or strong WiFi connection
- **GPU**: Optional but highly recommended for faster STT/TTS processing
  - NVIDIA GPU with CUDA support (e.g., GTX 1650 or better)
  - Alternative: Use CPU-optimized models (slower but functional)

### Software
- **Operating System**: Linux (Ubuntu 22.04+ recommended)
- **Docker**: For containerized deployment
- **Python 3.10+**: For STT/TTS model hosting
- **Node.js**: If xiaozhi-esp32-server requires it
- **NVIDIA Container Toolkit**: If using GPU acceleration

## Planned Features

### Vietnamese Speech-to-Text (STT)

Options under consideration:
1. **OpenAI Whisper** (multilingual, includes Vietnamese)
   - Excellent accuracy
   - Multiple model sizes (tiny → large)
   - Requires significant compute (GPU recommended)

2. **VAIS (Vietnamese AI Speech)** models
   - Optimized for Vietnamese language
   - May have better accuracy than multilingual models
   - Availability and licensing TBD

3. **Other Vietnamese ASR models**
   - Research ongoing for open-source options
   - Community suggestions welcome

### Vietnamese Text-to-Speech (TTS)

Options under consideration:
1. **VITS (Vietnamese TTS)**
   - Neural TTS for Vietnamese
   - Natural-sounding voice synthesis
   - Requires GPU for real-time performance

2. **Edge-TTS** (Microsoft Edge TTS)
   - Cloud-based (requires internet)
   - High quality Vietnamese voices
   - Free tier available

3. **Coqui TTS** with Vietnamese voice models
   - Open-source TTS framework
   - Community-trained Vietnamese models
   - Local processing

### Wake Word Detection

Implementing Vietnamese wake word detection is challenging and may require:
- Custom training on Vietnamese wake word dataset
- Lightweight model for on-device processing
- Possibly using OpenWakeWord or similar framework
- This is a stretch goal and may be deferred

## Development Roadmap

### Phase 3.1: Server Setup
- [ ] Set up Linux server (bare metal or VM)
- [ ] Install Docker and required dependencies
- [ ] Deploy xiaozhi-esp32-server
- [ ] Configure network and firewall
- [ ] Test basic server-client communication (English)

### Phase 3.2: Vietnamese STT Integration
- [ ] Research and select Vietnamese STT model
- [ ] Install and configure STT model
- [ ] Test STT accuracy with Vietnamese speech samples
- [ ] Optimize STT latency and performance
- [ ] Integrate STT with xiaozhi-esp32-server

### Phase 3.3: Vietnamese TTS Integration
- [ ] Research and select Vietnamese TTS model
- [ ] Install and configure TTS model
- [ ] Test TTS quality and naturalness
- [ ] Optimize TTS latency
- [ ] Integrate TTS with xiaozhi-esp32-server

### Phase 3.4: End-to-End Testing
- [ ] Configure ESP32-S3 device to use self-hosted server
- [ ] Test Vietnamese voice commands end-to-end
- [ ] Measure and optimize total latency (target: <2 seconds)
- [ ] Test various AC control commands in Vietnamese
- [ ] Handle errors and edge cases

### Phase 3.5: Wake Word (Future)
- [ ] Research Vietnamese wake word detection solutions
- [ ] Train or acquire wake word model
- [ ] Deploy wake word detection on device or server
- [ ] Test wake word accuracy and false positive rate

## Configuration

Configuration files will be added as development progresses. Expected structure:

```
server/
├── docker-compose.yml       # Docker deployment configuration
├── config/
│   ├── xiaozhi-server.json # Server configuration
│   ├── stt-config.yaml     # STT model configuration
│   └── tts-config.yaml     # TTS model configuration
├── models/                  # Downloaded AI models (gitignored)
├── scripts/
│   ├── setup.sh            # Initial setup script
│   ├── start.sh            # Start server services
│   └── stop.sh             # Stop server services
└── README.md               # This file
```

## Expected Usage

Once Phase 3 is complete, usage will be:

1. **Server Admin**: Deploy server, configure models
2. **Device Setup**: Configure ESP32-S3 to point to server IP
3. **Voice Control**: User speaks Vietnamese command
4. **Processing**: Server recognizes speech, processes intent, returns action
5. **Execution**: ESP32-S3 controls AC via IR based on server response

### Example Vietnamese Commands (Planned)

- "Bật máy lạnh" (Turn on AC)
- "Tắt máy lạnh" (Turn off AC)
- "Đặt nhiệt độ 25 độ" (Set temperature to 25°C)
- "Chế độ làm lạnh" (Cooling mode)
- "Tăng nhiệt độ" (Increase temperature)
- "Giảm nhiệt độ" (Decrease temperature)

## Performance Goals

| Metric | Target | Notes |
|--------|--------|-------|
| STT Latency | <1s | Time to transcribe voice command |
| TTS Latency | <1s | Time to synthesize voice response |
| Total Latency | <2s | End-to-end response time |
| STT Accuracy | >95% | For clear speech, quiet environment |
| TTS Naturalness | High | Natural-sounding Vietnamese voice |
| Wake Word Accuracy | >90% | If implemented |
| False Positives | <1/hour | Wake word false triggers |

## Deployment Options

### Option 1: Docker Compose (Recommended)
Containerized deployment for easy setup and portability.

### Option 2: Native Installation
Direct installation on Linux host for maximum performance.

### Option 3: Kubernetes
For advanced users with existing K8s infrastructure.

## Privacy and Security

- **Local Processing**: All voice data processed locally (no cloud)
- **Network Security**: Server accessible only on local network
- **Data Retention**: Voice recordings not stored by default
- **Encryption**: TLS/SSL for device-server communication (recommended)

## Cost Considerations

### Hardware
- Budget option: Repurpose existing PC ($0 if available)
- Mid-range: Used mini PC or Intel NUC (~$200-400)
- High-end: Dedicated server with GPU (~$500-1000+)

### Operating Costs
- **Electricity**: 24/7 server operation (~50-200W depending on hardware)
- **No cloud fees**: Self-hosted = no monthly subscription
- **Internet**: Local network only, no special internet requirements

## Challenges and Risks

### Model Quality
Vietnamese STT/TTS models may have varying quality. Thorough testing required to find the best open-source options.

### Latency
Real-time voice interaction requires fast processing. GPU acceleration or optimized models may be necessary.

### Wake Word Difficulty
Creating a reliable Vietnamese wake word detector is technically challenging and may not be feasible without significant research.

### Maintenance
Self-hosted server requires ongoing maintenance (updates, monitoring, troubleshooting).

## Resources

- [xiaozhi-esp32-server Repository](https://github.com/xinnan-tech/xiaozhi-esp32-server)
- [OpenAI Whisper](https://github.com/openai/whisper) - Multilingual STT
- [Coqui TTS](https://github.com/coqui-ai/TTS) - Open-source TTS framework
- [NVIDIA NeMo](https://github.com/NVIDIA/NeMo) - ASR/TTS toolkit
- [OpenWakeWord](https://github.com/dscripka/openWakeWord) - Wake word detection

## Contributing

Phase 3 development will begin once Phase 2 is complete and stable. Contributions are welcome:
- Share knowledge about Vietnamese STT/TTS models
- Test server deployment on different hardware configurations
- Contribute Vietnamese voice command datasets
- Improve documentation and setup scripts

---

**Last Updated**: October 26, 2026  
**Status**: Planning stage - awaiting Phase 2 completion before implementation.
