# EDU Revolution – AI-Powered Event Query Assistant

An ESP32-based event query assistant that answers attendee questions in real time using a fine-tuned GPT-4o-mini model, with an on-device TFT display and automated email escalation for unresolved queries.

## Overview

Attendees type a question via serial input, which is sent to a custom fine-tuned OpenAI model. The response is displayed live on an onboard TFT screen. If the attendee isn't satisfied with the answer, the system collects their registration number and automatically emails event staff with the full query and response for follow-up.

## System Components

| Component | Role |
|---|---|
| ESP32 | Main controller, WiFi + HTTP client |
| ST7735 TFT Display | Shows live question/response text |
| 4x Status LEDs | Visual state feedback (see below) |
| OpenAI API (fine-tuned GPT-4o-mini) | Answers attendee questions |
| Gmail SMTP | Sends escalation emails on dissatisfaction |

## Status LEDs

A 4-state visual feedback system:

| State | LED | Meaning |
|---|---|---|
| Waiting for input | Blue | System idle, ready for a question |
| Processing | Yellow | Query sent, awaiting API response |
| Response received | Green | Answer displayed successfully |
| Error / Dissatisfied | Red | API error, or user marked response unsatisfactory |

## How It Works

1. User types a question over Serial
2. Question is displayed on the TFT screen
3. Question is sent to a fine-tuned GPT-4o-mini model via the OpenAI Chat Completions API
4. Response is parsed from JSON and displayed on the TFT
5. User is prompted: satisfied with the response? (Y/N)
   - **Y** → system returns to idle, ready for the next question
   - **N** → user enters their registration number, and the system automatically emails event staff via Gmail SMTP with the registration number, original query, and the response given — flagged high priority for follow-up

## Tech Stack

- **Hardware**: ESP32, ST7735 TFT display (SPI)
- **Language**: C++ (Arduino framework)
- **AI**: OpenAI API, fine-tuned GPT-4o-mini model
- **Libraries**: `ArduinoJson` (response parsing), `ESP_Mail_Client` (SMTP escalation), `Adafruit_GFX` / `Adafruit_ST7735` (display)
- **Escalation**: Gmail SMTP (port 465)

## Setup

This project requires credentials that must **never** be committed to version control:

1. Copy `config.example.h` to `config.h`
2. Fill in your own values:
   - WiFi SSID / password
   - OpenAI API key (and your fine-tuned model ID, if using one)
   - Gmail sender address + [app password](https://support.google.com/accounts/answer/185833)
   - Admin/recipient email for escalations
3. `config.h` is gitignored — it will never be pushed

> ⚠️ If you ever accidentally commit an API key or password, treat it as compromised immediately — revoke/rotate it, don't just delete the commit (it stays in git history).

## Repository Structure
```
edu-revolution-ai-event-assistant/
├── src/
│   └── edumate.ino
├── config.example.h
├── .gitignore
└── README.md
```

## Demo

The system running live, showing a question about RPL (Recognition of Prior Learning) answered in real time on the TFT display, and the escalation flow triggering an email send:

*(screenshots/demo images to be added)*

## Status

Functional prototype — demonstrated live Q&A via fine-tuned model with on-device display and working email escalation.

## Author

**Kavya Sikri**
[LinkedIn](https://www.linkedin.com/in/kavya-sikri) | [GitHub](https://github.com/KVSIKRI)
