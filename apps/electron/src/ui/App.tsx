import { useState } from 'react'

function App() {
  const [bluetoothStatus] = useState('Scanning...')
  const [devices] = useState([
    { id: '1', name: 'BKMD Dongle', status: 'Connected', type: 'Keyboard' },
    { id: '2', name: 'Logitech MX Master', status: 'Paired', type: 'Mouse' },
  ])

  return (

    // AI UI - Jeste na tom nepracuji
    <div className="flex h-screen w-screen overflow-hidden bg-slate-950 text-slate-200">
      {/* Sidebar */}
      <aside className="w-64 bg-slate-900 border-r border-slate-800 flex flex-col">
        <div className="p-6">
          <h1 className="text-2xl font-bold text-blue-500 tracking-tight">BKMD</h1>
          <p className="text-xs text-slate-500 uppercase font-semibold mt-1">Control Center</p>
        </div>
        
        <nav className="flex-1 px-4 space-y-2">
          <button className="w-full text-left px-4 py-2 rounded-lg bg-slate-800 text-white font-medium transition-colors">
            Dashboard
          </button>
          <button className="w-full text-left px-4 py-2 rounded-lg hover:bg-slate-800 text-slate-400 hover:text-white transition-colors">
            Devices
          </button>
          <button className="w-full text-left px-4 py-2 rounded-lg hover:bg-slate-800 text-slate-400 hover:text-white transition-colors">
            Keylogs
          </button>
          <button className="w-full text-left px-4 py-2 rounded-lg hover:bg-slate-800 text-slate-400 hover:text-white transition-colors">
            Settings
          </button>
        </nav>


        {/* STATUS ikonka */}
        {/* <div className="p-4 border-t border-slate-800">
          <div className="flex items-center gap-3 px-2 py-1">
            <div className="w-2 h-2 rounded-full bg-green-500 animate-pulse"></div>
            <span className="text-sm font-medium">System Active</span>
          </div>
        </div> */}
      </aside>

      {/* Main Content */}
      <main className="flex-1 flex flex-col overflow-auto">
        {/* Top Header */}
        <header className="h-16 border-b border-slate-800 flex items-center justify-between px-8 bg-slate-900/50 backdrop-blur-md sticky top-0 z-10">
          <h2 className="text-lg font-semibold">Device Dashboard</h2>
          <div className="flex items-center gap-4">
            <span className="text-sm text-slate-400">BT Status: <span className="text-blue-400 font-mono">{bluetoothStatus}</span></span>
            <button className="bg-blue-600 hover:bg-blue-500 text-white px-4 py-1.5 rounded-md text-sm font-medium transition-colors shadow-lg shadow-blue-900/20">
              Refresh
            </button>
          </div>
        </header>

        {/* Dashboard Content */}
        <div className="p-8 space-y-8">

          {/* Device Table */}
          <section>
            <h3 className="text-xl font-bold mb-4">Nearby Devices</h3>
            <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden">
              <table className="w-full text-left">
                <thead className="bg-slate-800/50 border-b border-slate-800 text-xs uppercase text-slate-400">
                  <tr>
                    <th className="px-6 py-4 font-semibold tracking-wider">Device Name</th>
                    <th className="px-6 py-4 font-semibold tracking-wider">Type</th>
                    <th className="px-6 py-4 font-semibold tracking-wider">Status</th>
                    <th className="px-6 py-4 font-semibold tracking-wider text-right">Actions</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-slate-800">
                  {devices.map((device) => (
                    <tr key={device.id} className="hover:bg-slate-800/30 transition-colors">
                      <td className="px-6 py-4 font-medium">{device.name}</td>
                      <td className="px-6 py-4 text-slate-400 text-sm">{device.type}</td>
                      <td className="px-6 py-4">
                        <span className={`px-2 py-1 rounded-full text-[10px] font-bold uppercase ${
                          device.status === 'Connected' ? 'bg-green-500/10 text-green-500' : 'bg-slate-700 text-slate-400'
                        }`}>
                          {device.status}
                        </span>
                      </td>
                      <td className="px-6 py-4 text-right">
                        <button className="text-blue-400 hover:text-blue-300 text-sm font-medium">Configure</button>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </section>

        </div>
      </main>
    </div>
  )
}

export default App
