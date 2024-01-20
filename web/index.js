import React, { useState } from 'react';
import Tutorial from './tutorial';
import ReactDOM from 'react-dom';

function ReactApp() {
  const [count, setCount] = useState(0);

  return (
    <div id="reactAppRendered">
      <Tutorial />
    </div>
  );
}

ReactDOM.render(<ReactApp />, document.getElementById('reactApp'));