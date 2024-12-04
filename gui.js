import React, { useState, useEffect } from 'react';
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Alert, AlertDescription } from '@/components/ui/alert';
import { Car, Wifi, HeartPulse, Zap } from 'lucide-react';
import { Input } from '@/components/ui/input';

const SmartParkApp = () => {
  const [connected, setConnected] = useState(false);
  const [parkingSpots, setParkingSpots] = useState([
    { id: 1, type: 'normal', occupied: false, label: 'Standard Parking' },
    { id: 2, type: 'disabled', occupied: false, label: 'Disabled Parking' },
    { id: 3, type: 'charging', occupied: false, label: 'EV Charging' }
  ]);
  const [lastUpdate, setLastUpdate] = useState(null);
  const [mqttConfig, setMqttConfig] = useState({
    broker: 'broker.hivemq.com',
    port: '8000',
    topic: 'smartpark/spots/#'
  });

  const getSpotIcon = (type) => {
    switch(type) {
      case 'disabled':
        return <HeartPulse size={32} />; // Usando HeartPulse invece di Wheelchair
      case 'charging':
        return <Zap size={32} />; // Usando Zap invece di Battery
      default:
        return <Car size={32} />;
    }
  };

  const handleConnect = () => {
    setConnected(true);
    setLastUpdate(new Date().toLocaleString());
  };

  const handleConfigChange = (e) => {
    const { name, value } = e.target;
    setMqttConfig(prev => ({
      ...prev,
      [name]: value
    }));
  };

  return (
    <div className="p-4 max-w-4xl mx-auto">
      <Card className="mb-4">
        <CardHeader>
          <CardTitle>MQTT Configuration</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid gap-4">
            <div className="flex flex-col gap-2">
              <label>Broker Address:</label>
              <Input 
                name="broker"
                value={mqttConfig.broker}
                onChange={handleConfigChange}
                placeholder="es: broker.hivemq.com"
              />
            </div>
            <div className="flex flex-col gap-2">
              <label>Port:</label>
              <Input 
                name="port"
                value={mqttConfig.port}
                onChange={handleConfigChange}
                placeholder="es: 8000"
              />
            </div>
            <div className="flex flex-col gap-2">
              <label>Topic:</label>
              <Input 
                name="topic"
                value={mqttConfig.topic}
                onChange={handleConfigChange}
                placeholder="es: smartpark/spots/#"
              />
            </div>
            <Button 
              onClick={handleConnect}
              className={`mt-4 ${connected ? 'bg-green-600' : 'bg-blue-600'}`}
            >
              <Wifi size={18} className="mr-2" />
              {connected ? 'Connected' : 'Connect to MQTT'}
            </Button>
          </div>
        </CardContent>
      </Card>

      <Card className="mb-4">
        <CardHeader>
          <CardTitle>Smart Parking Status</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
            {parkingSpots.map((spot) => (
              <Card key={spot.id} className="p-4">
                <div className="flex flex-col items-center">
                  <div className={spot.occupied ? 'text-red-500' : 'text-green-500'}>
                    {getSpotIcon(spot.type)}
                  </div>
                  <p className="mt-2 font-medium">{spot.label}</p>
                  <p className="text-sm text-gray-500">
                    {spot.occupied ? 'Occupied' : 'Available'}
                  </p>
                </div>
              </Card>
            ))}
          </div>
          
          {connected && (
            <Alert className="mt-4">
              <AlertDescription>
                Last update: {lastUpdate || 'Waiting for data...'}
              </AlertDescription>
            </Alert>
          )}
        </CardContent>
      </Card>
    </div>
  );
};

export default SmartParkApp;
