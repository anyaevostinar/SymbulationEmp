import React, { useState } from 'react';
import Tutorial from './tutorial';
import ReactDOM from 'react-dom/client';
import { createRoot } from 'react-dom/client';

function App() {
  const [count, setCount] = useState(0);

  return (
    <div id="appRendered">
      <Tutorial />
    </div>
  );
}

//ReactDOM.render(<ReactApp />, document.getElementById('reactApp'));
const root = ReactDOM.createRoot(document.getElementById('app'));
root.render(<App />);