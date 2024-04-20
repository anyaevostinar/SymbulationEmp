/**
 * @name Copilot
 * @description React component that conditionally renders the Copilot window or the button that opens it
 * @returns {JSX.Element} a div containing the Copilot window or a button to open it
 */
import React, { useState, useEffect } from 'react';
import OpenButton from './components/OpenButton';
import { ThemeProvider } from '@mui/material/styles';
import muiTheme from './muiTheme';
import CopilotWindow from './CopilotWindow';
import "./copilot.css";

export default function Copilot() {
  const [copOpen, setCopOpen] = useState(false);
  // copOpen is a state that determines if the Copilot window is open. It is passed down to OpenButton and to CopilotWindow (which passes it down to CloseButton)
  const [displayOpenBtn, setDisplayOpenBtn] = useState(true);
  // displayOpenBtn is a state that determines if the button to open the Copilot window will be displayed
  const [displayCopWindow, setDisplayCopWindow] = useState(false);
  // displayCopWindow is a state that determines if the Copilot window will be displayed

  useEffect(() => {
    console.log = () => { };
    // making sure no stray console logs are shown in the final product; temporarily remove this line if you want to see console logs
  });

  function handleOpen() {
    setCopOpen(true);
    setTimeout(() => {
      setDisplayOpenBtn(false);
      setDisplayCopWindow(true);
    }, 150); // this is 150 because in copilot.scss, the fade in/out transitions are set to 150ms. ensure this number matches the transition duration
  }
  function handleClose() {
    setCopOpen(false);
    setTimeout(() => {
      setDisplayOpenBtn(true);
      setDisplayCopWindow(false);
    }, 150) // see above comment
  }
  return (
    <ThemeProvider theme={muiTheme}>
      {/* ThemeProvider is a Material-UI component that provides the theme to all components in Copilot. 
      right now, this is just for the buttons and their purple color. see muiTheme.js to change the color */}
      <div id='copilotParent'>
        {
          displayOpenBtn &&
          <OpenButton handleOpen={handleOpen} copOpen={copOpen} />
        }
        {
          displayCopWindow &&
          <CopilotWindow copOpen={copOpen} handleClose={handleClose} />
        }
      </div>
    </ThemeProvider>
  );
}