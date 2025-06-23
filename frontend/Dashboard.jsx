import React, { useState, useEffect, useCallback } from 'react';

const Dashboard = () => {
  const [processes, setProcesses] = useState([]);
  const [drivers, setDrivers] = useState([]);
  const [cheatEntries, setCheatEntries] = useState([]);
  
  const [selectedProcess, setSelectedProcess] = useState('');
  const [selectedDriver, setSelectedDriver] = useState('');
  const [dllPath, setDllPath] = useState('');
  
  const [message, setMessage] = useState('');
  const [messageType, setMessageType] = useState('');

  useEffect(() => {
    fetch('/api/processes')
      .then(res => res.json())
      .then(data => {
        setProcesses(data);
        if (data.length > 0) setSelectedProcess(data[0].pid);
      })
      .catch(err => {
        console.error("Error cargando procesos:", err);
        setProcesses([
          { pid: 1234, name: "notepad.exe" },
          { pid: 5678, name: "explorer.exe" },
          { pid: 9012, name: "chrome.exe" }
        ]);
        setSelectedProcess(1234);
      });

    fetch('/api/drivers')
      .then(res => res.json())
      .then(data => {
        setDrivers(data);
        if (data.length > 0) setSelectedDriver(data[0]);
      })
      .catch(err => {
        console.error("Error cargando drivers:", err);
        setMessage("Error al cargar la lista de drivers. Revisa que existan archivos .sys en la carpeta drivers/");
        setMessageType('error');
      });
  }, []);
  
  const handleFileChange = (event) => {
    const file = event.target.files[0];
    if (!file) return;

    const formData = new FormData();
    formData.append("ct_file", file);
    
    setMessage('Cargando tabla de trucos...');
    setMessageType('');

    fetch('/api/upload_ct', {
      method: 'POST',
      body: formData,
    })
    .then(res => {
      if(!res.ok) throw new Error("Fallo en la subida del archivo");
      return res.json();
    })
    .then(data => {
      setCheatEntries(data);
      setMessage(`Tabla '${file.name}' cargada con ${data.length} entradas.`);
      setMessageType('success');
    })
    .catch(err => {
      console.error("Error al cargar .ct:", err);
      setMessage("Error al procesar el archivo .ct. Verifica que sea un archivo XML válido.");
      setMessageType('error');
    });
  };
  
  const toggleCheatEntry = (index) => {
    const newEntries = [...cheatEntries];
    newEntries[index].active = !newEntries[index].active;
    setCheatEntries(newEntries);
  };

  const applyCheats = () => {
    const activeCheats = cheatEntries.filter(entry => entry.active);
    if (activeCheats.length === 0) {
      setMessage("No hay trucos activados para aplicar.");
      setMessageType("error");
      return;
    }
    
    const payload = {
      processId: selectedProcess,
      cheats: activeCheats
    };
    
    fetch('/api/apply_cheats', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    })
    .then(res => res.json())
    .then(data => {
      if (data.success) {
        setMessage(`${data.applied_count} trucos aplicados correctamente.`);
        setMessageType('success');
      } else {
        setMessage(data.error || "Error al aplicar trucos");
        setMessageType('error');
      }
    })
    .catch(err => {
      console.error("Error aplicando trucos:", err);
      setMessage("Error al aplicar trucos.");
      setMessageType('error');
    });
  };

  const injectDLL = () => {
    if (!dllPath || !selectedProcess) {
      setMessage("Debes seleccionar un proceso y especificar la ruta de la DLL.");
      setMessageType('error');
      return;
    }
    
    const payload = {
      pid: selectedProcess,
      dllPath: dllPath
    };
    
    fetch('/api/inject_dll', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    })
    .then(res => res.json())
    .then(data => {
      if (data.success) {
        setMessage(data.message);
        setMessageType('success');
      } else {
        setMessage(data.error || "Error al inyectar DLL");
        setMessageType('error');
      }
    })
    .catch(err => {
      console.error("Error inyectando DLL:", err);
      setMessage("Error al inyectar DLL.");
      setMessageType('error');
    });
  };

  return (
    <div className="container">
      <h1>Dashboard de Inyección Belzebub</h1>
      {message && <div className={`message ${messageType}`}>{message}</div>}
      
      <div className="main-controls">
        <div className="form-group">
          <label>Proceso Objetivo:</label>
          <select value={selectedProcess} onChange={e => setSelectedProcess(e.target.value)}>
            {processes.map(p => <option key={p.pid} value={p.pid}>{p.name} (PID: {p.pid})</option>)}
          </select>
        </div>
        <div className="form-group">
          <label>Driver a Utilizar:</label>
          <select value={selectedDriver} onChange={e => setSelectedDriver(e.target.value)}>
            {drivers.length > 0 ? drivers.map(d => <option key={d} value={d}>{d}</option>) : <option disabled>No hay drivers disponibles</option>}
          </select>
        </div>
      </div>

      <div className="feature-section">
        <h2>Inyección de DLL</h2>
        <div className="form-group">
          <label>Ruta de la DLL:</label>
          <input 
            type="text" 
            value={dllPath} 
            onChange={e => setDllPath(e.target.value)} 
            placeholder="C:\\ruta\\a\\payload.dll" 
          />
        </div>
        <button onClick={injectDLL}>Inyectar DLL</button>
      </div>
      
      <div className="feature-section">
        <h2>Manejo de Cheat Table (.ct)</h2>
        <div className="form-group">
          <label>Cargar Archivo .ct:</label>
          <input type="file" accept=".ct,.xml" onChange={handleFileChange} />
        </div>
        {cheatEntries.length > 0 && (
          <>
            <button onClick={applyCheats}>Aplicar Trucos Activados</button>
            <table className="cheat-table">
              <thead>
                <tr>
                  <th>Activar</th>
                  <th>Descripción</th>
                  <th>Dirección</th>
                  <th>Tipo</th>
                  <th>Valor</th>
                </tr>
              </thead>
              <tbody>
                {cheatEntries.map((entry, index) => (
                  <tr key={index}>
                    <td>
                      <input 
                        type="checkbox" 
                        checked={entry.active} 
                        onChange={() => toggleCheatEntry(index)} 
                      />
                    </td>
                    <td>{entry.description}</td>
                    <td>{entry.address}</td>
                    <td>{entry.type}</td>
                    <td>{entry.value}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </>
        )}
      </div>
      
      <div className="feature-section">
        <h2>Información del Sistema</h2>
        <div className="info-grid">
          <div className="info-item">
            <span className="info-label">Proceso Seleccionado:</span>
            <span className="info-value">{selectedProcess}</span>
          </div>
          <div className="info-item">
            <span className="info-label">Driver Seleccionado:</span>
            <span className="info-value">{selectedDriver}</span>
          </div>
          <div className="info-item">
            <span className="info-label">Drivers Disponibles:</span>
            <span className="info-value">{drivers.length}</span>
          </div>
          <div className="info-item">
            <span className="info-label">Cheats Cargados:</span>
            <span className="info-value">{cheatEntries.length}</span>
          </div>
        </div>
      </div>

      <style jsx>{`
        .container {
          max-width: 1200px;
          margin: 0 auto;
          padding: 20px;
          font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
          background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
          min-height: 100vh;
          color: white;
        }
        
        h1 {
          text-align: center;
          margin-bottom: 30px;
          font-size: 2.5em;
          text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        
        h2 {
          color: #fff;
          border-bottom: 2px solid rgba(255,255,255,0.3);
          padding-bottom: 10px;
          margin-bottom: 20px;
        }
        
        .message {
          padding: 15px;
          margin-bottom: 20px;
          border-radius: 5px;
          font-weight: bold;
        }
        
        .message.success {
          background-color: #4CAF50;
          color: white;
        }
        
        .message.error {
          background-color: #f44336;
          color: white;
        }
        
        .main-controls {
          display: grid;
          grid-template-columns: 1fr 1fr;
          gap: 20px;
          margin-bottom: 30px;
        }
        
        .feature-section {
          background: rgba(255,255,255,0.1);
          padding: 25px;
          margin-bottom: 25px;
          border-radius: 10px;
          backdrop-filter: blur(10px);
          border: 1px solid rgba(255,255,255,0.2);
        }
        
        .form-group {
          margin-bottom: 15px;
        }
        
        .form-group label {
          display: block;
          margin-bottom: 5px;
          font-weight: bold;
        }
        
        .form-group input, .form-group select {
          width: 100%;
          padding: 10px;
          border: none;
          border-radius: 5px;
          font-size: 16px;
          background: rgba(255,255,255,0.9);
          color: #333;
        }
        
        button {
          background: linear-gradient(45deg, #ff6b6b, #ee5a24);
          color: white;
          border: none;
          padding: 12px 25px;
          border-radius: 5px;
          cursor: pointer;
          font-size: 16px;
          font-weight: bold;
          transition: transform 0.2s;
          margin-right: 10px;
          margin-bottom: 10px;
        }
        
        button:hover {
          transform: translateY(-2px);
          box-shadow: 0 5px 15px rgba(0,0,0,0.3);
        }
        
        .cheat-table {
          width: 100%;
          border-collapse: collapse;
          margin-top: 20px;
          background: rgba(255,255,255,0.9);
          border-radius: 10px;
          overflow: hidden;
          color: #333;
        }
        
        .cheat-table th, .cheat-table td {
          padding: 12px;
          text-align: left;
          border-bottom: 1px solid rgba(0,0,0,0.1);
        }
        
        .cheat-table th {
          background: rgba(0,0,0,0.1);
          font-weight: bold;
        }
        
        .cheat-table tr:hover {
          background: rgba(0,0,0,0.05);
        }
        
        .info-grid {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
          gap: 15px;
        }
        
        .info-item {
          background: rgba(255,255,255,0.1);
          padding: 15px;
          border-radius: 8px;
          display: flex;
          justify-content: space-between;
          align-items: center;
        }
        
        .info-label {
          font-weight: bold;
        }
        
        .info-value {
          background: rgba(255,255,255,0.2);
          padding: 5px 10px;
          border-radius: 15px;
          font-family: monospace;
        }
      `}</style>
    </div>
  );
};

export default Dashboard;
