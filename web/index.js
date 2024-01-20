import React, { useState } from 'react';
import ReactDOM from 'react-dom';

function Counter() {
  const [count, setCount] = useState(0);

  return (
    <div>
      <p>Count: {count}</p>
      <button onClick={() => setCount(count + 1)}>
        Increment
      </button>
    </div>
  );
}

ReactDOM.render(<Counter />, document.getElementById('react'));