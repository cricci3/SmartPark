# Smart Park Dashboard 🚗

> A modern real-time parking management system powered by Next.js and MQTT

![TypeScript](https://img.shields.io/badge/TypeScript-007ACC?style=for-the-badge&logo=typescript&logoColor=white)
![Next.js](https://img.shields.io/badge/Next.js-000000?style=for-the-badge&logo=nextdotjs&logoColor=white)
![MQTT](https://img.shields.io/badge/MQTT-660066?style=for-the-badge&logo=mqtt&logoColor=white)
![Tailwind CSS](https://img.shields.io/badge/Tailwind_CSS-38B2AC?style=for-the-badge&logo=tailwind-css&logoColor=white)

## 🌟 Overview

Transform your parking management with a sleek, real-time dashboard that monitors parking spot availability across multiple floors. Built with modern web technologies and designed for seamless real-time updates.

## ✨ Features

🔄 **Real-time Monitoring**
- Instant updates on parking spot availability
- Live connection status indicators
- Comprehensive event logging

🏢 **Smart Floor Management**
- Multi-floor support
- Intuitive visual layout
- At-a-glance status overview

🅿️ **Spot Categories**
- `Standard` Regular parking spots
- `Disabled` Accessible parking spots
- `EV` Electric vehicle charging stations

## 🚀 Quick Start

### Prerequisites

- Node.js 18.x or later
- npm or yarn package manager
- MQTT broker access

### Setup

```bash
# Clone the repository
git clone <repository-url>

# Navigate to project directory
cd smart-park-dashboard

# Install dependencies
npm install

# Start the development server
npm run dev
```

Visit [http://localhost:3000](http://localhost:3000) to see your dashboard in action!

## 🔌 MQTT Configuration

### Default Settings
```javascript
{
  protocol: 'WSS',
  broker: 'test.mosquitto.org',
  port: 8081,
  topics: [
    'parking/floor0',  // Ground floor
    'parking/floor1'   // First floor
  ]
}
```

### Message Format
```javascript
"0,0,1"  // Format: [standard,disabled,ev]
// 0 = available, 1 = occupied
```

## 🛠️ Development

```plaintext
smart-park-dashboard/
├── app/
│   └── page.tsx          # Main dashboard
├── components/
│   ├── mqtt-config       # MQTT settings
│   ├── floor-section     # Floor display
│   └── parking-spot      # Spot indicators
└── public/
    └── assets/          # Static resources
```

## 🤝 Contributing

1. 🍴 Fork the repository
2. 🌿 Create your branch (`git checkout -b feature/amazing`)
3. 💭 Commit changes (`git commit -m 'Add amazing feature'`)
4. 🚀 Push to branch (`git push origin feature/amazing`)
5. 🔄 Open a Pull Request

## 📜 License

Released under the MIT License. See [LICENSE](LICENSE) for details.

## 💫 Acknowledgments

Built with:
- ⚡ Next.js for lightning-fast performance
- 🔄 MQTT.js for reliable real-time updates
- 🎨 Tailwind CSS for modern styling

---

<div align="center">

Made with ❤️ for better parking management

[Report Bug](https://github.com/yourusername/smart-park-dashboard/issues) · [Request Feature](https://github.com/yourusername/smart-park-dashboard/issues)

</div>