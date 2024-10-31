import React from 'react';
import './Tabs.css';

function Tabs({ tabs }) {
  const [activeTab, setActiveTab] = React.useState(Object.keys(tabs)[0]);

  return (
    <div className="tabs-container">
      <div className="tabs-header">
        {Object.keys(tabs).map((tabKey) => (
          <div
            key={tabKey}
            className={`tab ${activeTab === tabKey ? 'active' : ''}`}
            onClick={() => setActiveTab(tabKey)}
          >
            {tabKey.charAt(0).toUpperCase() + tabKey.slice(1)}
          </div>
        ))}
      </div>
      <div className="tabs-content">
        {tabs[activeTab]}
      </div>
    </div>
  );
}

export default Tabs;
