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
  //const [hasToggleBeenClicked, setHasToggleBeenClicked] = useState(false);
  useEffect(() => {
    let toggleButton;
    setTimeout(() => {
      toggleButton = document.getElementById('toggle');
      toggleButton.addEventListener('click', () => {
        setHasToggleBeenClicked(true);
      });
    }, 100);
  }, []);
  function handleOpen() {
    setCopOpen(true);
    setTimeout(() => {
      setDisplayOpenBtn(false);
      setDisplayCopWindow(true);
    }, 150); // change from 150 to whatever the transition time is in copilot.scss
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
          <CopilotWindow copOpen={copOpen} />
        }
      </div>
    </ThemeProvider>
  );
}