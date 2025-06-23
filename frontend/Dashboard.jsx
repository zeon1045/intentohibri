import React, { useState, useEffect } from 'react';
import axios from 'axios';

const Dashboard = () => {
  const [status, setStatus] = useState('Cargando...');
  const [scripts, setScripts] = useState([]);

  useEffect(() => {
    axios.get('/api/status')
      .then(response => setStatus(response.data.status))
      .catch(() => setStatus('Error al cargar'));

    axios.get('/api/scripts')
      .then(response => setScripts(response.data.scripts))
      .catch(() => setScripts([]));
  }, []);

  const toggleScript = (index) => {
    const updated = scripts.map((s, i) => i === index ? { ...s, active: !s.active } : s);
    setScripts(updated);
    axios.post('/api/toggleScript', { index, active: updated[index].active })
      .catch(() => alert('Error al actualizar el script'));
  };

  return (
    <div className="bg-gray-900 text-white min-h-screen p-8">
      <h1 className="text-3xl mb-4">Panel de Control Belzebub</h1>
      <div className="mb-8">
        <h2 className="text-xl">Estado del Motor</h2>
        <p>{status}</p>
      </div>
      <div>
        <h2 className="text-xl">Scripts disponibles</h2>
        {scripts.length > 0 ? (
          <ul>
            {scripts.map((script, idx) => (
              <li key={idx} className="py-2 border-b border-gray-700 flex justify-between items-center">
                <span>{script.name} - {script.active ? 'Activado' : 'Desactivado'}</span>
                <button
                  className={`px-3 py-1 rounded ${script.active ? 'bg-red-500' : 'bg-green-500'}`}
                  onClick={() => toggleScript(idx)}>
                  {script.active ? 'Desactivar' : 'Activar'}
                </button>
              </li>
            ))}
          </ul>
        ) : (
          <p>No hay scripts disponibles.</p>
        )}
      </div>
    </div>
  );
};

export default Dashboard;
