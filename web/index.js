import React, { useState } from 'react';
import Tutorial from './Copilot';
import ReactDOM from 'react-dom/client';
import { createRoot } from 'react-dom/client';

function ReactApp() {
  return (
    <div id="appRendered">
      <Tutorial />
    </div>
  );
}

//ReactDOM.render(<ReactApp />, document.getElementById('reactApp'));
const root = ReactDOM.createRoot(document.getElementById('reactApp'));
root.render(<ReactApp />);