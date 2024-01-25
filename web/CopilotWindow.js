import React, { useState, useEffect } from 'react';
import CloseIcon from '@mui/icons-material/Close';
import CloseButton from './components/CloseButton';

export default function CopilotWindow({ copOpen, handleClose }) {
  return (
    <>
      {copOpen ? (
        <div id="copWindow" className='fade-in'>
          <p>i am the copwindow component</p>
          <CloseButton handleClose={handleClose} copOpen={copOpen}/>
        </div>
      ) : (
        <div id="copWindow" className='fade-out'>
          <p>i am the copwindow component</p>
          <CloseButton handleClose={handleClose} copOpen={copOpen} />
        </div>
      )}
    </>
  );
}