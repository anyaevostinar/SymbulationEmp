/**
 * @name NoneInnerHTML
 * @description React component for a tutorial step that contains a title and body text, where the tutorial step's body's innerHTML is set to the bodyText
 * This is useful for when the bodyText contains HTML tags that need to be rendered (e.g. <a> tags)
 * @param {object} content an object containing the tutorial step's title text and body text
 * content will be passed down from CopilotWindow.js, which gets it from tutorials.js
 * @param {string} content.titleText the title text for the tutorial step
 * @param {string} content.bodyText the body text for the tutorial step
 * @returns {JSX.Element} a div containing the title and body text for the tutorial step, where the body text's innerHTML is set to the bodyText
 * Note: this component doesn't have buttons to advance the tutorial, so it's necessary to include an advanceOn in the tutorial step's object in tutorials.js
 */
import React from "react";

export default function NoneInnerHTML({content}) {
  return(
    <div className="none">
      <h2 className="tutorialTitle">{content.titleText}</h2>
      <p className="tutorialDescription" dangerouslySetInnerHTML={{__html: content.bodyText}}></p>
    </div>
  );
}