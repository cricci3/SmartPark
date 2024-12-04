'use client';
import React, { useState } from 'react';
import { Car, Wifi, Zap } from 'lucide-react';

export default function Home() {
  const [connected, setConnected] = useState(false);
  const [parkingSpots, setParkingSpots] = useState([
    { id: 1, type: 'normal', occupied: false, label: 'Standard Parking' },
    { id: 2, type: 'disabled', occupied: false, label: 'Disabled Parking' },
    { id: 3, type: 'charging', occupied: false, label: 'EV Charging' }
  ]);

  const getSpotIcon = (type: string) => {
    switch(type) {
      case 'disabled':
        return <Car className="h-8 w-8" />;
      case 'charging':
        return <Zap className="h-8 w-8" />;
      default:
        return <Car className="h-8 w-8" />;
    }
  };

  return (
    <div className="min-h-screen bg-white">
      <main className="max-w-5xl mx-auto p-8">
        {/* Header */}
        <h1 className="text-3xl font-bold text-gray-800 mb-8">Smart Park Dashboard</h1>

        {/* MQTT Configuration Card */}
        <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-6 mb-8 hover:shadow-md transition-shadow">
          <div className="flex items-center justify-between mb-6">
            <h2 className="text-xl font-semibold text-gray-800">MQTT Configuration</h2>
            <button 
              onClick={() => setConnected(!connected)}
              className={`
                flex items-center gap-2 px-6 py-2.5 rounded-lg text-white 
                transition-all duration-200 
                ${connected ? 'bg-green-500 hover:bg-green-600' : 'bg-blue-500 hover:bg-blue-600'}
              `}
            >
              <Wifi className="h-4 w-4" />
              {connected ? 'Connected' : 'Connect'}
            </button>
          </div>
          
          <div className="grid md:grid-cols-2 gap-6">
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">Broker Address</label>
              <input 
                type="text"
                className="w-full p-3 rounded-lg border border-gray-200 focus:border-blue-500 focus:ring-1 focus:ring-blue-500 outline-none transition-all"
                placeholder="broker.hivemq.com"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">Port</label>
              <input 
                type="text"
                className="w-full p-3 rounded-lg border border-gray-200 focus:border-blue-500 focus:ring-1 focus:ring-blue-500 outline-none transition-all"
                placeholder="8000"
              />
            </div>
          </div>
        </div>

        {/* Parking Spots Grid */}
        <div className="grid md:grid-cols-3 gap-6">
          {parkingSpots.map((spot) => (
            <div 
              key={spot.id} 
              className={`
                bg-white rounded-xl p-6 
                border border-gray-100
                hover:shadow-md transition-all duration-200
                ${spot.occupied ? 'hover:border-red-100' : 'hover:border-green-100'}
              `}
            >
              <div className={`
                flex items-center justify-center h-16 w-16 rounded-full mx-auto mb-4
                ${spot.occupied ? 'bg-red-50 text-red-500' : 'bg-green-50 text-green-500'}
              `}>
                {getSpotIcon(spot.type)}
              </div>
              <h3 className="text-lg font-semibold text-gray-800 text-center mb-2">
                {spot.label}
              </h3>
              <p className={`
                text-sm text-center px-4 py-1.5 rounded-full mx-auto w-fit
                ${spot.occupied ? 
                  'bg-red-50 text-red-600' : 
                  'bg-green-50 text-green-600'}
              `}>
                {spot.occupied ? 'Occupied' : 'Available'}
              </p>
            </div>
          ))}
        </div>

        {/* Status Bar */}
        {connected && (
          <div className="mt-8 text-sm text-gray-500 flex items-center gap-2">
            <div className="h-2 w-2 rounded-full bg-green-500"></div>
            Last update: {new Date().toLocaleTimeString()}
          </div>
        )}
      </main>
    </div>
  );
}