import React, { useState, useEffect } from 'react';
import OpenButton from './components/OpenButton';
import { ThemeProvider } from '@mui/material/styles';
import muiTheme from './muiTheme';
import CopilotWindow from './CopilotWindow';
import "./copilot.css";

export default function Copilot() {
  const [copOpen, setCopOpen] = useState(false);
  const [displayOpenBtn, setDisplayOpenBtn] = useState(true);
  const [displayCopWindow, setDisplayCopWindow] = useState(false);

  useEffect(() => {
    console.log = () => {};
    // making sure no stray console logs are left in the final product
  });
  

  function handleOpen() {
    setCopOpen(true);
    setTimeout(() => {
      setDisplayOpenBtn(false);
      setDisplayCopWindow(true);
    }, 150); // change from 150 to whatever the transition time is in copilot.scss
  }
  function handleClose(){
    setCopOpen(false);
    setTimeout(() => {
      setDisplayOpenBtn(true);
      setDisplayCopWindow(false);
    }, 150)
  }


  return (
    <ThemeProvider theme={muiTheme}>
      <div id='copilotParent'>
        {
          displayOpenBtn && 
          <OpenButton handleOpen={handleOpen} copOpen={copOpen} />
        }
        {
          displayCopWindow && 
          <CopilotWindow copOpen={copOpen} handleClose={handleClose}/>
        }
      </div>
    </ThemeProvider>
  );
}