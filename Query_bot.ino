#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // JSON parsing
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <ESP_Mail_Client.h>  // For sending emails

// WiFi Credentialswh
const char* ssid = //Your Device Name;
const char* password = //Password;

// OpenAI API Token
const char* chatgpt_token = // Your Chat GPT API Token-Key

String chatgpt_Q;  // User query
String user_response;  // User satisfaction response
String reg_number; // User registration number

// TFT SPI Pins
#define TFT_CS     5   // Chip Select
#define TFT_RST    4   // Reset
#define TFT_DC     2   // Data/Command

// TFT Display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Gmail SMTP Credentials
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

const char* sender_email = // Your Gmail
const char* sender_password = // App password
const char* admin_email = // Email where queries will be sent

ESP_Mail_Session session;
SMTP_Message message;

// LED Pins
#define BLUE_LED 33   // Waiting for input
#define YELLOW_LED 13 // Processing query
#define GREEN_LED 14  // Response received
#define RED_LED 25    // Error or dissatisfaction

// Function Prototypes
void sendEmail(String reg_number, String query, String response);
void displayOnTFT(String title, String content);
void setupLEDs();
void setupTFT();

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    setupLEDs();
    setupTFT();

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi Connected!");

    tft.setCursor(10, 10);
    tft.println("EduMate Ready:\nHow may i help you?");
}

void setupLEDs() {
    pinMode(BLUE_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);

    digitalWrite(BLUE_LED, HIGH);  // Initial state: Waiting for input
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
}

void setupTFT() {
    tft.initR(INITR_BLACKTAB);
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
}

void displayOnTFT(String title, String content) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(5, 10);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.println(title + ":");

    int yPos = 30;
    for (int i = 0; i < content.length(); i += 18) {
        tft.setCursor(5, yPos);
        tft.println(content.substring(i, i + 18));
        yPos += 15;
    }
}

void sendEmail(String reg_number, String query, String response) {
    SMTPSession smtp;
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = sender_email;
    session.login.password = sender_password;
    session.login.user_domain = "gmail.com";

    message.sender.name = "ESP32 Query Bot";
    message.sender.email = sender_email;
    message.subject = "User Query Dissatisfaction Report";
    message.addRecipient("Admin", admin_email);
    
    // Email content
    message.text.content = "Registration No: " + reg_number + "\nQuery: " + query + "\nResponse: " + response + "\n Please contact the student and look into the matter.";
    message.text.charSet = "utf-8";
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;

    displayOnTFT("Status", "Sending Email...\n Sorry for Trouble\n Officials will contact soon.");
    Serial.println("Sending dissatisfaction report email...");

    if (!smtp.connect(&session)) {
        Serial.println("❌ SMTP Connection Failed!");
        return;
    }

    if (!MailClient.sendMail(&smtp, &message)) {
        Serial.println("❌ Email Send Failed: " + smtp.errorReason());
    } else {
        Serial.println("✅ Dissatisfaction Report Sent Successfully!");
        displayOnTFT("Status", "Mail Sent Successfully!");
    }

    smtp.closeSession();
}

void loop() {
    if (Serial.available()) {
        digitalWrite(BLUE_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);

        chatgpt_Q = Serial.readStringUntil('\n');
        Serial.println("\nSending question to ChatGPT: " + chatgpt_Q);

        // Display Question on TFT
        displayOnTFT("User Question", chatgpt_Q);

        HTTPClient https;
        if (https.begin("https://api.openai.com/v1/chat/completions")) {
            https.addHeader("Content-Type", "application/json");
            String token_key = "Bearer " + String(chatgpt_token);
            https.addHeader("Authorization", token_key);
            https.useHTTP10(true);

        String payload = String("{\"model\": \"ft:gpt-4o-mini-2024-07-18:kavya:hackathon:BEGwnPtJ\", \"messages\": [{\"role\": \"user\", \"content\": \"") 
    + chatgpt_Q + String("\"}], \"temperature\": 0.7, \"max_tokens\": 30}");


            int httpCode = https.POST(payload);

            if (httpCode > 0) {
                digitalWrite(YELLOW_LED, LOW);
                digitalWrite(GREEN_LED, HIGH);

                String response = https.getString();
                Serial.println("Response received:");
                Serial.println(response);

                DynamicJsonDocument doc(1024);
                deserializeJson(doc, response);
                String chatgpt_answer = doc["choices"][0]["message"]["content"].as<String>();

                displayOnTFT("EduMate Response", chatgpt_answer);

                Serial.println("\nSatisfied with the response? (Y/N)");
                
                while (!Serial.available());
                user_response = Serial.readStringUntil('\n');
                user_response.trim();

                if (user_response.equalsIgnoreCase("N")) {
                    digitalWrite(GREEN_LED, LOW);
                    digitalWrite(RED_LED, HIGH);
                    Serial.println("Please enter your registration number:");
                    displayOnTFT("Dissatisfied!", "Enter Registration No:");

                    while (!Serial.available());
                    reg_number = Serial.readStringUntil('\n');
                    reg_number.trim();

                    sendEmail(reg_number, chatgpt_Q, chatgpt_answer);

                    delay(2000);
                    digitalWrite(RED_LED, LOW);
                }
            } else {
                digitalWrite(YELLOW_LED, LOW);
                digitalWrite(RED_LED, HIGH);
                delay(2000);
                digitalWrite(RED_LED, LOW);
            }
            https.end();
        }

        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, HIGH);
        Serial.println("\nType your next question:");
    }
}