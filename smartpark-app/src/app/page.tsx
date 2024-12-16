"use client";
import React, { useState, useEffect } from "react";
import { Car, Wifi, Zap, Accessibility } from "lucide-react";
import mqtt, { MqttClient } from "mqtt";

interface ParkingSpot {
  id: number;
  type: "normal" | "disabled" | "charging";
  occupied: boolean;
  label: string;
}

interface FloorSectionProps {
  floor: string;
  spots: ParkingSpot[];
}

interface ParkingFloors {
  floor0: ParkingSpot[];
  floor1: ParkingSpot[];
}

export default function Home() {
  const [client, setClient] = useState<MqttClient | null>(null);
  const [connected, setConnected] = useState(false);
  const [lastUpdate, setLastUpdate] = useState<string | null>(null);
  const [logs, setLogs] = useState<string[]>([]);
  interface MqttConfig {
    protocol: "ws" | "wss";
    hostname: string;
    port: number;
    path: string;
    clientId: string;
    keepalive: number;
  }

  const [showLogs, setShowLogs] = useState(false);
  const [mqttConfig, setMqttConfig] = useState<MqttConfig>({
    protocol: "wss",
    hostname: "test.mosquitto.org",
    port: 8081,
    path: "/mqtt",
    clientId: `parking-dashboard-${Math.random()
      .toString(16)
      .substring(2, 10)}`,
    keepalive: 60,
  });

  // Inizializza i parcheggi per entrambi i piani
  const [parkingFloors, setParkingFloors] = useState<ParkingFloors>({
    floor0: [
      { id: 1, type: "normal", occupied: false, label: "Standard Parking" },
      { id: 2, type: "disabled", occupied: false, label: "Disabled Parking" },
      { id: 3, type: "charging", occupied: false, label: "EV Charging" },
    ],
    floor1: [
      { id: 1, type: "normal", occupied: false, label: "Standard Parking" },
      { id: 2, type: "disabled", occupied: false, label: "Disabled Parking" },
      { id: 3, type: "charging", occupied: false, label: "EV Charging" },
    ],
  });

  const addLog = (message: string) => {
    const timestamp = new Date().toLocaleTimeString();
    setLogs((prev) => [`${timestamp}: ${message}`, ...prev.slice(0, 9)]);
  };

  const connectToMqtt = () => {
    try {
      const connectUrl = `${mqttConfig.protocol}://${mqttConfig.hostname}:${mqttConfig.port}${mqttConfig.path}`;
      addLog(`Attempting to connect to ${connectUrl}`);

      const options = {
        clientId: mqttConfig.clientId,
        keepalive: mqttConfig.keepalive,
        clean: true,
        reconnectPeriod: 5000,
      };

      const mqttClient = mqtt.connect(connectUrl, options);
      setClient(mqttClient);

      mqttClient.on("connect", () => {
        setConnected(true);
        addLog("Successfully connected to MQTT broker");

        // Subscribe to both floor topics
        mqttClient.subscribe("parking/floor0", (err) => {
          if (!err) {
            addLog("Subscribed to parking/floor0");
          } else {
            addLog(`Subscription error for floor0: ${err.message}`);
          }
        });

        mqttClient.subscribe("parking/floor1", (err) => {
          if (!err) {
            addLog("Subscribed to parking/floor1");
          } else {
            addLog(`Subscription error for floor1: ${err.message}`);
          }
        });
      });

      mqttClient.on("message", (topic, message) => {
        try {
          const rawMessage = message.toString();
          addLog(`Raw message received on ${topic}: ${rawMessage}`);

          const values = rawMessage.split(",").map((v) => parseInt(v.trim()));

          if (values.length === 3) {
            // Determina quale piano aggiornare in base al topic
            const floor = topic.endsWith("floor0") ? "floor0" : "floor1";

            setParkingFloors((prev) => ({
              ...prev,
              [floor]: prev[floor].map((spot, index) => ({
                ...spot,
                occupied: values[index] === 1,
              })),
            }));

            addLog(`Updated ${floor} states: ${values.join(",")}`);
            setLastUpdate(new Date().toLocaleTimeString());
          } else {
            addLog(
              `Invalid message format for ${topic}. Expected 3 values, got ${values.length}`
            );
          }
        } catch (error) {
          addLog(`Error processing message: ${error}`);
          addLog(`Failed message content: "${message.toString()}"`);
        }
      });

      mqttClient.on("error", (err) => {
        addLog(`Connection error: ${err.message}`);
        setConnected(false);
      });

      mqttClient.on("close", () => {
        addLog("Connection closed");
        setConnected(false);
      });

      mqttClient.on("offline", () => {
        addLog("Client went offline");
        setConnected(false);
      });
    } catch (error) {
      addLog(`Connection attempt failed: ${error}`);
    }
  };

  const disconnectMqtt = () => {
    if (client) {
      addLog("Disconnecting from MQTT broker");
      client.end();
      setConnected(false);
      setClient(null);
    }
  };

  const getSpotIcon = (type: string) => {
    switch (type) {
      case "disabled":
        return <Accessibility className="h-8 w-8" />;
      case "charging":
        return <Zap className="h-8 w-8" />;
      default:
        return <Car className="h-8 w-8" />;
    }
  };

  useEffect(() => {
    return () => {
      if (client) {
        client.end();
      }
    };
  }, [client]);

  const FloorSection: React.FC<FloorSectionProps> = ({ floor, spots }) => (
    <div className="mb-8">
      <h2 className="text-xl font-bold text-gray-800 mb-4">
        Floor {floor.replace("floor", "")}
      </h2>
      <div className="grid md:grid-cols-3 gap-6">
        {spots.map((spot) => (
          <div
            key={spot.id}
            className={`
              bg-white rounded-xl p-6 
              border border-gray-100
              hover:shadow-md transition-all duration-200
              ${
                spot.occupied
                  ? "hover:border-red-100"
                  : "hover:border-green-100"
              }
            `}
          >
            <div
              className={`
              flex items-center justify-center h-16 w-16 rounded-full mx-auto mb-4
              ${
                spot.occupied
                  ? "bg-red-50 text-red-500"
                  : "bg-green-50 text-green-500"
              }
            `}
            >
              {getSpotIcon(spot.type)}
            </div>
            <h3 className="text-lg font-semibold text-gray-800 text-center mb-2">
              {spot.label}
            </h3>
            <p
              className={`
              text-sm text-center px-4 py-1.5 rounded-full mx-auto w-fit
              ${
                spot.occupied
                  ? "bg-red-50 text-red-600"
                  : "bg-green-50 text-green-600"
              }
            `}
            >
              {spot.occupied ? "Occupied" : "Available"}
            </p>
          </div>
        ))}
      </div>
    </div>
  );

  return (
    <div className="min-h-screen bg-white">
      <main className="max-w-5xl mx-auto p-8">
        <h1 className="text-3xl font-bold text-gray-800 mb-8">
          Smart Park Dashboard
        </h1>

        {/* MQTT Configuration Card */}
        <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-8 mb-8">
          <div className="flex items-center justify-between mb-8">
            <h2 className="text-xl font-semibold text-gray-900">
              MQTT Configuration
            </h2>
            <button
              onClick={() => (connected ? disconnectMqtt() : connectToMqtt())}
              className={`
                flex items-center gap-2 px-6 py-2.5 rounded-lg text-white 
                transition-all duration-200 
                ${
                  connected
                    ? "bg-green-500 hover:bg-green-600"
                    : "bg-blue-500 hover:bg-blue-600"
                }
              `}
            >
              <Wifi className="h-4 w-4" />
              {connected ? "Connected" : "Connect"}
            </button>
          </div>

          <div className="grid md:grid-cols-2 gap-12">
            {/* Connection Settings */}
            <div>
              <h3 className="font-medium text-gray-900 text-lg mb-6">
                Connection Settings
              </h3>
              <div className="space-y-6">
                <div>
                  <label className="block text-sm font-medium text-gray-800 mb-2">
                    Protocol
                  </label>
                  <select
                    className="w-full p-3 rounded-lg border border-gray-300 focus:border-blue-500 focus:ring-1 focus:ring-blue-500 outline-none transition-all text-gray-900 bg-white"
                    value={mqttConfig.protocol}
                    onChange={(e) =>
                      setMqttConfig((prev) => ({
                        ...prev,
                        protocol: e.target.value as "ws" | "wss",
                      }))
                    }
                    disabled={connected}
                  >
                    <option value="ws">WS</option>
                    <option value="wss">WSS</option>
                  </select>
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-800 mb-2">
                    Broker Address
                  </label>
                  <input
                    type="text"
                    className="w-full p-3 rounded-lg border border-gray-300 focus:border-blue-500 focus:ring-1 focus:ring-blue-500 outline-none transition-all text-gray-900 placeholder-gray-500"
                    value={mqttConfig.hostname}
                    onChange={(e) =>
                      setMqttConfig((prev) => ({
                        ...prev,
                        hostname: e.target.value,
                      }))
                    }
                    disabled={connected}
                    placeholder="test.mosquitto.org"
                  />
                </div>
              </div>
            </div>

            {/* Advanced Settings */}
            <div>
              <h3 className="font-medium text-gray-900 text-lg mb-6">
                Advanced Settings
              </h3>
              <div className="space-y-6">
                <div>
                  <label className="block text-sm font-medium text-gray-800 mb-2">
                    Client ID
                  </label>
                  <input
                    type="text"
                    className="w-full p-3 rounded-lg border border-gray-300 focus:border-blue-500 focus:ring-1 focus:ring-blue-500 outline-none transition-all text-gray-900 placeholder-gray-500"
                    value={mqttConfig.clientId}
                    onChange={(e) =>
                      setMqttConfig((prev) => ({
                        ...prev,
                        clientId: e.target.value,
                      }))
                    }
                    disabled={connected}
                    placeholder="Auto-generated"
                  />
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-800 mb-2">
                    Port
                  </label>
                  <input
                    type="number"
                    className="w-full p-3 rounded-lg border border-gray-300 focus:border-blue-500 focus:ring-1 focus:ring-blue-500 outline-none transition-all text-gray-900 placeholder-gray-500"
                    value={mqttConfig.port}
                    onChange={(e) =>
                      setMqttConfig((prev) => ({
                        ...prev,
                        port: parseInt(e.target.value) || prev.port,
                      }))
                    }
                    disabled={connected}
                    placeholder="8081"
                  />
                </div>
              </div>
            </div>
          </div>
        </div>

        {/* Parking Floors */}
        <FloorSection floor="floor0" spots={parkingFloors.floor0} />
        <FloorSection floor="floor1" spots={parkingFloors.floor1} />
        {/* Logs Section */}
        <div className="bg-white rounded-xl border border-gray-100">
          <div
            className="p-4 flex items-center justify-between cursor-pointer hover:bg-gray-50 transition-colors"
            onClick={() => setShowLogs(!showLogs)}
          >
            <h3 className="text-lg font-semibold text-gray-900">
              Connection Logs
            </h3>
            <button className="text-gray-500 hover:text-gray-700">
              {showLogs ? "Hide" : "Show"}
            </button>
          </div>
          {showLogs && (
            <div className="border-t border-gray-100">
              <div className="bg-gray-50 rounded-b-lg p-4 h-48 overflow-auto">
                {logs.map((log, index) => (
                  <div
                    key={index}
                    className="text-sm text-gray-600 mb-1 font-mono"
                  >
                    {log}
                  </div>
                ))}
                {logs.length === 0 && (
                  <div className="text-sm text-gray-400 text-center py-4">
                    No logs available
                  </div>
                )}
              </div>
            </div>
          )}
        </div>
        {/* Status Bar */}
        {connected && lastUpdate && (
          <div className="mt-8 text-sm text-gray-500 flex items-center gap-2">
            <div className="h-2 w-2 rounded-full bg-green-500"></div>
            Last update: {lastUpdate}
          </div>
        )}
      </main>
    </div>
  );
}
