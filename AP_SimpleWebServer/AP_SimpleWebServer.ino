#include <WiFi.h>
#include "arduino_secrets.h"
#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include <esp_task_wdt.h>
#include <atomic>
#include <mutex>

#define WDT_TIMEOUT 15     // Watchdog timeout in seconds

// Network credentials from arduino_secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// Server and connection status
int status = WL_IDLE_STATUS;
WiFiServer server(80);

// Atomic counter for active threads
std::atomic<int> activeThreads(0);

// Thread controller to manage threads
ThreadController threadController = ThreadController();

typedef struct {
    int standard;
    int handicap;
    int echarge;
} ParkingStalls;

volatile ParkingStalls floors[2];

// Structure to hold request data
struct RequestThread {
    String data;
    Thread thread;
    static std::mutex watchdogMutex;  // Declare mutex
    
    RequestThread(String d) : data(d) {
        thread.onRun([this]() {
            this->processRequest();
        });
        thread.setInterval(0);
    }
    
    void processRequest() {
        std::lock_guard<std::mutex> lock(watchdogMutex);

        int previousCount = activeThreads++;
        if(previousCount == 0) {  // first thread
            esp_task_wdt_add(NULL);
        }
        esp_task_wdt_reset();
        
        noInterrupts();
        handleJson(data);
        printFloorStatus();  
        interrupts();
        
        if(--activeThreads == 0) {  // We're the last thread
            esp_task_wdt_delete(NULL);
        }
        thread.enabled = false;
    }
};

// Define the static mutex
std::mutex RequestThread::watchdogMutex;

// Function prototypes
void printWiFiStatus();
void printFloorStatus();
void handleJson(String json);

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ; // Wait for serial port to connect
    }

    Serial.println("Multithread Server with Smart Watchdog");

    // Initialize Watchdog
    esp_task_wdt_init(WDT_TIMEOUT, true);
    
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        while (true);
    }

    WiFi.config(IPAddress(10, 0, 0, 1));
    
    Serial.print("Creating access point named: ");
    Serial.println(ssid);
    
    status = WiFi.beginAP(ssid, pass);
    if (status != WL_AP_LISTENING) {
        Serial.println("Creating access point failed");
        while (true);
    }

    server.begin();
    printWiFiStatus();

    // Initialize parking floors
    for (int i = 0; i < 2; i++) {
        floors[i].standard = 1;
        floors[i].handicap = 1;
        floors[i].echarge = 1;
    }

    printFloorStatus();
}

void loop() {
    // Monitor WiFi status
    if (status != WiFi.status()) {
        status = WiFi.status();
        if (status == WL_AP_CONNECTED) {
            Serial.println("Device connected to AP");
        } else {
            Serial.println("Device disconnected from AP");
        }
    }

    // Handle incoming clients
    WiFiClient client = server.accept();
    if (client) {
        Serial.println("New client connected");
        
        // Read the request
        String currentLine = "";
        String lastLine = "";
        unsigned long clientStartTime = millis();
        
        while (client.connected()) {
            if (millis() - clientStartTime > 5000) { // 5 second timeout
                break;
            }
            
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                clientStartTime = millis();
                
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.println();
                        break;
                    } else {
                        lastLine = currentLine;
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }

        // If we got data, create a thread to handle it
        if (lastLine.length() > 0) {
            RequestThread* request = new RequestThread(lastLine);
            if (request != nullptr) {
                threadController.add(&(request->thread));
            } else {
                Serial.println("Failed to create request thread");
            }
        }

        client.stop();
    }

    // Run thread controller
    if (threadController.size() > 0) {
        threadController.run();
    }

    // Clean up completed threads
    for (int i = 0; i < threadController.size(); i++) {
        Thread* t = threadController.get(i);
        if (t && !t->enabled) {
            RequestThread* rt = (RequestThread*)((char*)t - offsetof(RequestThread, thread));
            threadController.remove(i);
            delete rt;
            i--;
        }
    }

    delay(10);
}

bool isWatchdogActive() {
    return activeThreads > 0;
}

void printWiFiStatus() {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}

void printFloorStatus() {
    Serial.println("Current status:");
    for (int i = 0; i < 2; i++) {
        Serial.print("Parking Lot ");
        Serial.println(i);
        
        Serial.print("Available Car Stalls ");
        Serial.println(floors[i].standard);
        
        Serial.print("Available Handicap Stalls ");
        Serial.println(floors[i].handicap);
        
        Serial.print("Available E-Charge Stalls ");
        Serial.println(floors[i].echarge);
        
        Serial.println();
    }
}

void handleJson(String json) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }
    
    int floor_id = doc["floor_id"];
    int stall_type = doc["stall_type"];
    bool status = doc["status"];
    
    Serial.print("floor_id: ");
    Serial.println(floor_id);
    Serial.print("stall_type: ");
    Serial.println(stall_type);
    Serial.print("status: ");
    Serial.println(status);
    
    if (floor_id >= 0 && floor_id < 2) {
        switch (stall_type) {
            case 0:
                if(status)
                    floors[floor_id].standard = floors[floor_id].standard + 1;
                else
                    floors[floor_id].standard = floors[floor_id].standard - 1; 
                break;
            case 1:
                if(status)
                    floors[floor_id].handicap = floors[floor_id].handicap + 1;
                else
                    floors[floor_id].handicap = floors[floor_id].handicap - 1; 
                break;
            case 2:
                if(status)
                    floors[floor_id].echarge = floors[floor_id].echarge + 1;
                else
                    floors[floor_id].echarge = floors[floor_id].echarge - 1; 
                break;
            default:
                Serial.println("Invalid stall_type");
        }
    } else {
        Serial.println("Invalid floor_id");
    }
}