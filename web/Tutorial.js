import React, { useState } from 'react';

export default function Tutorial() {
  const [count, setCount] = useState(0);

  return (
    <div className='tutorial'>
      <p>Count: {count}</p>
      <button onClick={() => setCount(count + 1)}>
        INCREMENTT
      </button>
    </div>
  );
}