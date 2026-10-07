#pragma once

#include <string>

namespace UI {

inline const char* INDEX_HTML = R"rawhtml(<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>zested launcher</title>
  <style>
    :root {
      --bg-gradient: radial-gradient(circle at 50% -20%, #1e1b4b 0%, #0d0f18 60%, #08090d 100%);
      --surface-glass: rgba(255, 255, 255, 0.035);
      --surface-glass-border: rgba(255, 255, 255, 0.07);
      --surface-card: rgba(18, 21, 31, 0.7);
      --surface-card-hover: rgba(28, 32, 50, 0.85);
      --primary: #6366f1;
      --primary-hover: #4f46e5;
      --primary-glow: rgba(99, 102, 241, 0.35);
      --text-main: #f8fafc;
      --text-secondary: #94a3b8;
      --text-muted: #64748b;
      --accent: #38bdf8;
      --success: #10b981;
      --danger: #ef4444;
      --radius-sm: 8px;
      --radius-md: 14px;
      --radius-lg: 20px;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      user-select: none;
      -webkit-user-drag: none;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
      scroll-behavior: smooth;
    }

    html {
      scroll-behavior: smooth;
      background: transparent;
    }

    body {
      background: #08090d;
      background-image: var(--bg-gradient);
      color: var(--text-main);
      height: 100vh;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      border: 1px solid rgba(255, 255, 255, 0.08);
      border-radius: 12px;
      scroll-behavior: smooth;
    }

    /* Custom Titlebar (Window Drag Area) */
    .titlebar {
      height: 44px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 16px;
      background: rgba(13, 15, 24, 0.75);
      backdrop-filter: blur(25px);
      border-bottom: 1px solid var(--surface-glass-border);
      cursor: grab;
      z-index: 1000;
      border-top-left-radius: 12px;
      border-top-right-radius: 12px;
    }

    .titlebar:active {
      cursor: grabbing;
    }

    .titlebar-brand {
      display: flex;
      align-items: center;
      gap: 10px;
      font-size: 13.5px;
      font-weight: 700;
      letter-spacing: 0.8px;
      color: #f1f5f9;
      pointer-events: none;
    }

    .brand-icon-box {
      width: 24px;
      height: 24px;
      background: linear-gradient(135deg, #6366f1, #38bdf8);
      border-radius: 6px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 12px;
      color: white;
      box-shadow: 0 2px 8px rgba(99, 102, 241, 0.4);
    }

    .brand-badge {
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid rgba(255, 255, 255, 0.1);
      color: var(--accent);
      font-size: 10.5px;
      font-weight: 600;
      padding: 2px 8px;
      border-radius: 20px;
    }

    .titlebar-actions {
      display: flex;
      gap: 6px;
      cursor: default;
    }

    .win-action-btn {
      background: transparent;
      border: none;
      color: var(--text-secondary);
      font-size: 13px;
      cursor: pointer;
      width: 28px;
      height: 28px;
      display: flex;
      align-items: center;
      justify-content: center;
      border-radius: 6px;
      transition: all 0.15s ease;
    }

    .win-action-btn:hover {
      background: rgba(255, 255, 255, 0.1);
      color: #fff;
    }

    .win-action-btn.btn-close:hover {
      background: #ef4444;
      color: #fff;
    }

    /* Main Container */
    .app-container {
      flex: 1;
      display: flex;
      overflow: hidden;
    }

    /* Sidebar */
    .sidebar {
      width: 240px;
      background: rgba(13, 15, 24, 0.55);
      backdrop-filter: blur(30px);
      border-right: 1px solid var(--surface-glass-border);
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      padding: 22px 14px;
    }

    .nav-group {
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .nav-label {
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 1px;
      color: var(--text-muted);
      padding: 8px 12px 4px 12px;
    }

    .nav-item {
      display: flex;
      align-items: center;
      gap: 12px;
      padding: 11px 14px;
      border-radius: var(--radius-md);
      color: var(--text-secondary);
      font-size: 13.5px;
      font-weight: 500;
      cursor: pointer;
      border: 1px solid transparent;
      transition: background-color 0.15s ease, border-color 0.15s ease, color 0.15s ease, box-shadow 0.15s ease;
      position: relative;
    }

    .nav-item:hover {
      background: rgba(255, 255, 255, 0.05);
      color: #fff;
      transform: none;
    }

    .nav-item:active {
      background: rgba(255, 255, 255, 0.08);
      transform: none;
    }

    .nav-item.active {
      background: linear-gradient(90deg, rgba(99, 102, 241, 0.22) 0%, rgba(99, 102, 241, 0.06) 100%);
      color: #fff;
      font-weight: 500;
      border: 1px solid rgba(99, 102, 241, 0.35);
    }

    .nav-item.active::before {
      content: '';
      position: absolute;
      left: 0;
      top: 25%;
      bottom: 25%;
      width: 3.5px;
      background: var(--primary);
      border-radius: 0 4px 4px 0;
      box-shadow: 0 0 10px var(--primary);
    }

    .nav-icon {
      width: 18px;
      height: 18px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 14px;
      opacity: 0.9;
    }

    /* User Profile Card */
    .user-profile-card {
      background: var(--surface-card);
      border: 1px solid var(--surface-glass-border);
      border-radius: var(--radius-md);
      padding: 12px;
      display: flex;
      align-items: center;
      gap: 12px;
      cursor: pointer;
      transition: all 0.2s;
    }

    .user-profile-card:hover {
      border-color: rgba(99, 102, 241, 0.4);
      background: var(--surface-card-hover);
    }

    .avatar-img {
      width: 36px;
      height: 36px;
      border-radius: 8px;
      image-rendering: pixelated;
      background: #1e1b4b;
      box-shadow: 0 2px 6px rgba(0,0,0,0.4);
    }

    .user-info {
      flex: 1;
      overflow: hidden;
    }

    .user-name {
      font-size: 13.5px;
      font-weight: 600;
      white-space: nowrap;
      text-overflow: ellipsis;
      overflow: hidden;
      color: #f1f5f9;
    }

    .user-type {
      font-size: 11px;
      color: var(--text-muted);
      display: flex;
      align-items: center;
      gap: 5px;
      margin-top: 2px;
    }

    .type-dot {
      width: 6px;
      height: 6px;
      border-radius: 50%;
      background: var(--success);
      box-shadow: 0 0 6px var(--success);
    }

    /* Content Area */
    .content-area {
      flex: 1;
      overflow-y: auto;
      padding: 30px 40px;
      display: flex;
      flex-direction: column;
      gap: 26px;
    }

    .content-area::-webkit-scrollbar {
      width: 6px;
    }
    .content-area::-webkit-scrollbar-thumb {
      background: rgba(255, 255, 255, 0.1);
      border-radius: 3px;
    }

    .tab-content {
      display: none;
      flex-direction: column;
      gap: 26px;
      animation: fadeIn 0.25s ease-out;
    }

    .tab-content.active {
      display: flex;
    }

    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(6px); }
      to { opacity: 1; transform: translateY(0); }
    }

    /* Hero / Launch Section */
    .hero-banner {
      background: linear-gradient(135deg, rgba(30, 27, 75, 0.85) 0%, rgba(15, 23, 42, 0.85) 100%);
      border: 1px solid var(--surface-glass-border);
      border-radius: var(--radius-lg);
      padding: 36px;
      position: relative;
      overflow: hidden;
      display: flex;
      justify-content: space-between;
      align-items: center;
      box-shadow: 0 12px 35px rgba(0, 0, 0, 0.4);
    }

    .hero-banner::before {
      content: '';
      position: absolute;
      top: -60%;
      right: -20%;
      width: 500px;
      height: 500px;
      background: radial-gradient(circle, rgba(99, 102, 241, 0.25) 0%, transparent 70%);
      border-radius: 50%;
      pointer-events: none;
    }

    .hero-left {
      max-width: 520px;
      z-index: 1;
    }

    .hero-title {
      font-size: 30px;
      font-weight: 800;
      margin-bottom: 8px;
      background: linear-gradient(90deg, #ffffff, #cbd5e1);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }

    .hero-desc {
      font-size: 14px;
      color: var(--text-secondary);
      line-height: 1.5;
      margin-bottom: 20px;
    }

    .instance-badge-row {
      display: flex;
      gap: 10px;
      align-items: center;
      margin-bottom: 12px;
    }

    .tag-badge {
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid rgba(255, 255, 255, 0.12);
      padding: 4px 12px;
      border-radius: 20px;
      font-size: 11.5px;
      font-weight: 600;
      color: #e2e8f0;
      display: flex;
      align-items: center;
      gap: 6px;
    }

    .hero-right {
      z-index: 1;
      display: flex;
      flex-direction: column;
      align-items: flex-end;
      gap: 12px;
    }

    .play-btn {
      background: linear-gradient(135deg, #6366f1 0%, #4338ca 100%);
      border: none;
      color: white;
      font-size: 16.5px;
      font-weight: 700;
      letter-spacing: 0.5px;
      padding: 16px 46px;
      border-radius: var(--radius-md);
      cursor: pointer;
      box-shadow: 0 6px 25px var(--primary-glow);
      transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
      display: flex;
      align-items: center;
      gap: 12px;
    }

    .play-btn:hover {
      transform: translateY(-2px) scale(1.02);
      box-shadow: 0 10px 30px rgba(99, 102, 241, 0.55);
      background: linear-gradient(135deg, #4f46e5 0%, #3730a3 100%);
    }

    .play-btn:active {
      transform: translateY(1px);
    }

    .play-btn:disabled {
      background: #334155;
      color: #94a3b8;
      box-shadow: none;
      cursor: not-allowed;
      transform: none;
    }

    /* Progress bar in Hero */
    .launch-progress-box {
      width: 100%;
      margin-top: 18px;
      display: none;
    }

    .progress-bar-bg {
      width: 100%;
      height: 6px;
      background: rgba(255, 255, 255, 0.1);
      border-radius: 4px;
      overflow: hidden;
      margin-bottom: 6px;
    }

    .progress-bar-fill {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, #6366f1, #38bdf8);
      border-radius: 4px;
      transition: width 0.3s ease;
    }

    .progress-text {
      font-size: 12px;
      color: var(--text-secondary);
      display: flex;
      justify-content: space-between;
    }

    /* Cards Grid */
    .section-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 16px;
    }

    .section-title {
      font-size: 18.5px;
      font-weight: 700;
      color: #f1f5f9;
    }

    .grid-cards {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(290px, 1fr));
      gap: 18px;
    }

    .card {
      background: var(--surface-card);
      border: 1px solid var(--surface-glass-border);
      border-radius: var(--radius-md);
      padding: 20px;
      cursor: pointer;
      transition: all 0.25s cubic-bezier(0.4, 0, 0.2, 1);
      display: flex;
      flex-direction: column;
      gap: 14px;
      position: relative;
    }

    .card:hover {
      background: var(--surface-card-hover);
      border-color: rgba(99, 102, 241, 0.4);
      transform: translateY(-3px);
      box-shadow: 0 10px 24px rgba(0, 0, 0, 0.35);
    }

    .card.active {
      border-color: var(--primary);
      background: rgba(99, 102, 241, 0.12);
    }

    .card-top {
      display: flex;
      align-items: center;
      gap: 14px;
    }

    .card-icon {
      width: 48px;
      height: 48px;
      border-radius: 12px;
      background: rgba(255, 255, 255, 0.05);
      object-fit: cover;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 22px;
    }

    .card-meta {
      flex: 1;
      overflow: hidden;
    }

    .card-title {
      font-size: 15px;
      font-weight: 600;
      color: #f8fafc;
      white-space: nowrap;
      text-overflow: ellipsis;
      overflow: hidden;
    }

    .card-subtitle {
      font-size: 12px;
      color: var(--text-muted);
      margin-top: 3px;
    }

    .card-desc {
      font-size: 12.5px;
      color: var(--text-secondary);
      line-height: 1.45;
      display: -webkit-box;
      -webkit-line-clamp: 2;
      -webkit-box-orient: vertical;
      overflow: hidden;
    }

    .card-actions {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding-top: 10px;
      border-top: 1px solid rgba(255, 255, 255, 0.06);
    }

    /* Buttons */
    .btn {
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid var(--surface-glass-border);
      color: #f1f5f9;
      font-size: 12.5px;
      font-weight: 600;
      padding: 8px 16px;
      border-radius: var(--radius-sm);
      cursor: pointer;
      transition: all 0.15s ease;
      display: inline-flex;
      align-items: center;
      gap: 6px;
    }

    .btn:hover {
      background: rgba(255, 255, 255, 0.15);
      border-color: rgba(255, 255, 255, 0.2);
    }

    .btn-primary {
      background: var(--primary);
      border-color: var(--primary);
      color: white;
    }
    .btn-primary:hover {
      background: var(--primary-hover);
      border-color: var(--primary-hover);
    }

    .btn-danger {
      background: rgba(239, 68, 68, 0.15);
      color: #fca5a5;
      border-color: rgba(239, 68, 68, 0.3);
    }
    .btn-danger:hover {
      background: rgba(239, 68, 68, 0.3);
      color: #fff;
    }

    /* Search Input */
    .search-bar {
      display: flex;
      gap: 12px;
      margin-bottom: 20px;
    }

    .search-input {
      flex: 1;
      background: var(--surface-card);
      border: 1px solid var(--surface-glass-border);
      border-radius: var(--radius-md);
      padding: 12px 18px;
      font-size: 14px;
      color: #fff;
      outline: none;
      transition: all 0.2s;
    }

    .search-input:focus {
      border-color: var(--primary);
      box-shadow: 0 0 12px var(--primary-glow);
    }

    /* Settings Form */
    .form-group {
      display: flex;
      flex-direction: column;
      gap: 6px;
      margin-bottom: 18px;
    }

    .form-label {
      font-size: 13px;
      font-weight: 600;
      color: #cbd5e1;
    }

    .form-subtext {
      font-size: 11.5px;
      color: var(--text-muted);
    }

    .form-input, .form-select {
      background: var(--surface-card);
      border: 1px solid var(--surface-glass-border);
      border-radius: var(--radius-sm);
      padding: 10px 14px;
      font-size: 13.5px;
      color: #fff;
      outline: none;
      transition: all 0.2s;
    }

    .form-input:focus, .form-select:focus {
      border-color: var(--primary);
    }

    .form-select option {
      background: #0f172a;
      color: #fff;
    }

    .slider-container {
      display: flex;
      align-items: center;
      gap: 16px;
    }

    .form-slider {
      flex: 1;
      accent-color: var(--primary);
      cursor: pointer;
    }

    /* Modals */
    .modal-overlay {
      position: fixed;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background: rgba(0, 0, 0, 0.75);
      backdrop-filter: blur(10px);
      z-index: 2000;
      display: none;
      align-items: center;
      justify-content: center;
      animation: fadeIn 0.2s ease-out;
    }

    .modal-box {
      background: #11131f;
      border: 1px solid var(--surface-glass-border);
      border-radius: var(--radius-lg);
      width: 490px;
      max-width: 90%;
      padding: 28px;
      display: flex;
      flex-direction: column;
      gap: 18px;
      box-shadow: 0 20px 45px rgba(0, 0, 0, 0.65);
    }

    .modal-title {
      font-size: 18.5px;
      font-weight: 700;
      color: #fff;
    }

    .modal-actions {
      display: flex;
      justify-content: flex-end;
      gap: 10px;
      margin-top: 10px;
    }

    /* Notification Toast */
    .toast-container {
      position: fixed;
      bottom: 24px;
      right: 24px;
      z-index: 3000;
      display: flex;
      flex-direction: column;
      gap: 10px;
    }

    .toast {
      background: #1e1b4b;
      border: 1px solid rgba(99, 102, 241, 0.4);
      padding: 12px 18px;
      border-radius: var(--radius-md);
      box-shadow: 0 10px 25px rgba(0, 0, 0, 0.5);
      font-size: 13px;
      font-weight: 500;
      color: #fff;
      display: flex;
      align-items: center;
      gap: 10px;
      animation: slideIn 0.3s ease-out;
    }

    @keyframes slideIn {
      from { transform: translateX(100%); opacity: 0; }
      to { transform: translateX(0); opacity: 1; }
    }

    /* Floating Save Changes Bar in Settings */
    .settings-save-bar {
      position: fixed;
      bottom: 24px;
      right: 40px;
      left: 280px;
      background: rgba(18, 21, 31, 0.95);
      border: 1px solid rgba(99, 102, 241, 0.45);
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.6), 0 0 20px rgba(99, 102, 241, 0.25);
      border-radius: var(--radius-md);
      padding: 14px 20px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      backdrop-filter: blur(25px);
      z-index: 100;
      opacity: 0;
      pointer-events: none;
      transform: translateY(20px);
      transition: all 0.25s cubic-bezier(0.4, 0, 0.2, 1);
    }

    .settings-save-bar.visible {
      opacity: 1;
      pointer-events: auto;
      transform: translateY(0);
    }

    .settings-save-info {
      display: flex;
      align-items: center;
      gap: 10px;
      font-size: 13.5px;
      font-weight: 600;
      color: #f1f5f9;
    }

    .settings-save-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: #f59e0b;
      box-shadow: 0 0 8px #f59e0b;
    }
  </style>
</head>
<body>

  <!-- Frameless Custom Titlebar -->
  <div class="titlebar" id="drag-titlebar">
    <div class="titlebar-brand">
      <div class="brand-icon-box">&#9670;</div>
      <span>zested launcher</span>
    </div>
    <div class="titlebar-actions">
      <button class="win-action-btn" title="Свернуть" onclick="minWindow()">&#x2014;</button>
      <button class="win-action-btn btn-close" title="Закрыть" onclick="closeWindow()">&#x2715;</button>
    </div>
  </div>

  <div class="app-container">
    <!-- Sidebar Navigation -->
    <div class="sidebar">
      <div class="nav-group">
        <div class="nav-label">Навигация</div>
        <div class="nav-item active" onclick="switchTab('home')">
          <span class="nav-icon">&#9658;</span>
          <span>Главная</span>
        </div>
        <div class="nav-item" onclick="switchTab('instances')">
          <span class="nav-icon">&#9638;</span>
          <span>Мои сборки</span>
        </div>
        <div class="nav-item" onclick="switchTab('modpacks')">
          <span class="nav-icon">&#10022;</span>
          <span>Каталог сборок</span>
        </div>
        <div class="nav-item" onclick="switchTab('mods')">
          <span class="nav-icon">&#9881;</span>
          <span>Каталог модов</span>
        </div>
        <div class="nav-item" onclick="switchTab('versions')">
          <span class="nav-icon">&#128196;</span>
          <span>Все версии MC</span>
        </div>

        <div class="nav-label" style="margin-top: 14px;">Конфигурация</div>
        <div class="nav-item" onclick="switchTab('settings')">
          <span class="nav-icon">&#9874;</span>
          <span>Параметры</span>
        </div>
      </div>

      <!-- Account profile card -->
      <div class="user-profile-card" onclick="openAccountsModal()">
        <img id="user-avatar" class="avatar-img" src="https://minotar.net/helm/Player/100.png" alt="Avatar">
        <div class="user-info">
          <div id="user-name" class="user-name">Player</div>
          <div class="user-type">
            <span class="type-dot"></span>
            <span id="user-type-text">Offline</span>
          </div>
        </div>
        <span style="color: var(--text-muted); font-size: 14px;">&#9656;</span>
      </div>
    </div>

    <!-- Main Content Area -->
    <div class="content-area">

      <!-- TAB: HOME -->
      <div id="tab-home" class="tab-content active">
        <!-- Hero Launch Section -->
        <div class="hero-banner">
          <div class="hero-left">
            <div class="instance-badge-row">
              <span id="hero-tag-ver" class="tag-badge">Minecraft 1.21.1</span>
              <span id="hero-tag-loader" class="tag-badge">Fabric Loader</span>
            </div>
            <h1 id="hero-title" class="hero-title">Default 1.21.1</h1>
            <p id="hero-desc" class="hero-desc">Оптимизированная сборка с высокой производительностью и полной поддержкой модов.</p>
            <div id="launch-progress" class="launch-progress-box">
              <div class="progress-bar-bg">
                <div id="progress-fill" class="progress-bar-fill"></div>
              </div>
              <div class="progress-text">
                <span id="progress-status-text">Подготовка файлов...</span>
                <span id="progress-percent-text">0%</span>
              </div>
            </div>
          </div>
          <div class="hero-right" style="display: flex; flex-direction: column; align-items: stretch; gap: 8px;">
            <button id="btn-play" class="play-btn" onclick="launchActiveInstance()">
              <span>&#9658;</span>
              <span>ИГРАТЬ</span>
            </button>
            <button class="btn" style="width: 100%; justify-content: center; font-size: 13px;" onclick="openActiveInstanceSettings()">&#9881; Настроить сборку</button>
          </div>
        </div>

        <!-- Quick Instances List -->
        <div>
          <div class="section-header">
            <h2 class="section-title">Быстрый выбор сборки</h2>
            <button class="btn" onclick="openCreateInstanceModal()">+ Создать сборку</button>
          </div>
          <div id="quick-instances" class="grid-cards">
            <!-- Injected by JS -->
          </div>
        </div>
      </div>

      <!-- TAB: INSTANCES -->
      <div id="tab-instances" class="tab-content">
        <div class="section-header">
          <div>
            <h2 class="section-title">Управление сборками</h2>
            <div style="font-size: 13px; color: var(--text-secondary); margin-top: 4px;">
              Создавайте собственные уникальные конфигурации или настраивайте существующие
            </div>
          </div>
          <button class="btn btn-primary" onclick="openCreateInstanceModal()">+ Новая сборка</button>
        </div>
        <div id="instances-list" class="grid-cards">
          <!-- Injected by JS -->
        </div>
      </div>

      <!-- TAB: MODPACKS -->
      <div id="tab-modpacks" class="tab-content">
        <div class="section-header">
          <div>
            <h2 class="section-title">Каталог готовых сборок</h2>
            <div style="font-size: 13px; color: var(--text-secondary); margin-top: 4px;">
              Загружайте топовые модпаки в один клик
            </div>
          </div>
        </div>
        <div class="search-bar">
          <input id="modpack-search" type="text" class="search-input" placeholder="Поиск модпаков (например: Fabulously Optimized, Cobblemon...)" onkeydown="if(event.key==='Enter') searchModpacks()">
          <button class="btn btn-primary" onclick="searchModpacks()">Найти</button>
        </div>
        <div id="modpacks-list" class="grid-cards">
          <!-- Injected by JS -->
        </div>
      </div>

      <!-- TAB: MODS -->
      <div id="tab-mods" class="tab-content">
        <div class="section-header">
          <div>
            <h2 class="section-title">Каталог модов</h2>
            <div style="font-size: 13px; color: var(--text-secondary); margin-top: 4px;">
              Устанавливайте моды напрямую в активную сборку
            </div>
          </div>
        </div>
        <div class="search-bar">
          <input id="mod-search" type="text" class="search-input" placeholder="Поиск модов (например: Sodium, Iris, Lithium, JEI...)" onkeydown="if(event.key==='Enter') searchMods()">
          <button class="btn btn-primary" onclick="searchMods()">Найти</button>
        </div>
        <div id="mods-list" class="grid-cards">
          <!-- Injected by JS -->
        </div>
      </div>

      <!-- TAB: VERSIONS -->
      <div id="tab-versions" class="tab-content">
        <div class="section-header">
          <div>
            <h2 class="section-title">Официальные версии Minecraft</h2>
            <div style="font-size: 13px; color: var(--text-secondary); margin-top: 4px;">
              Создайте профиль любой релизной версии
            </div>
          </div>
        </div>
        <div id="versions-list" class="grid-cards">
          <!-- Injected by JS -->
        </div>
      </div>

      <!-- TAB: SETTINGS -->
      <div id="tab-settings" class="tab-content">
        <div class="section-header">
          <div>
            <h2 class="section-title">Параметры и оптимизация</h2>
            <div style="font-size: 13px; color: var(--text-secondary); margin-top: 4px;">
              Выделение оперативной памяти, версия Java и параметры запуска
            </div>
          </div>
        </div>

        <div style="max-width: 680px; display: flex; flex-direction: column; gap: 16px;">
          <div class="form-group">
            <label class="form-label">Выделение оперативной памяти (RAM)</label>
            <div id="ram-subtext" class="form-subtext">Рекомендуется от 4096 МБ (4 ГБ) для плавной игры с модами</div>
            <div class="slider-container" style="margin-top: 8px;">
              <input id="setting-ram" type="range" class="form-slider" min="1024" max="16384" step="512" oninput="updateRamLabel(this.value); checkSettingsModified()">
              <span id="ram-val" style="font-weight: 700; width: 90px; text-align: right; color: var(--accent);">4096 MB</span>
            </div>
          </div>

          <div class="form-group">
            <label class="form-label">Исполняемый файл Java</label>
            <div class="form-subtext">Путь к javaw.exe (обнаруженные версии в системе или свой путь)</div>
            <div style="display: flex; gap: 8px; margin-top: 6px;">
              <select id="setting-java" class="form-select" style="flex: 1;" onchange="checkSettingsModified()"></select>
              <button class="btn" onclick="browseJava()">Обзор...</button>
            </div>
          </div>

          <div class="form-group">
            <label class="form-label">Оптимизированные аргументы JVM</label>
            <div class="form-subtext">Флаги сборщика мусора G1GC для устранения микрофризов</div>
            <input id="setting-jvm" type="text" class="form-input" oninput="checkSettingsModified()">
          </div>

          <div class="form-group">
            <label class="form-label">Разрешение экрана игры</label>
            <div style="display: flex; gap: 10px; align-items: center; flex-wrap: wrap;">
              <input id="setting-res-w" type="number" class="form-input" style="width: 100px;" placeholder="1280" oninput="checkSettingsModified()">
              <span>&times;</span>
              <input id="setting-res-h" type="number" class="form-input" style="width: 100px;" placeholder="720" oninput="checkSettingsModified()">
              <button class="btn" style="padding: 4px 10px; font-size: 11.5px;" onclick="setResPreset(1280, 720)">720p</button>
              <button class="btn" style="padding: 4px 10px; font-size: 11.5px;" onclick="setResPreset(1920, 1080)">1080p</button>
              <button id="btn-native-res" class="btn" style="padding: 4px 10px; font-size: 11.5px;" onclick="setNativeRes()">Монитор</button>
              <label style="display: flex; align-items: center; gap: 8px; margin-left: 8px; font-size: 13px; cursor: pointer;">
                <input id="setting-fullscreen" type="checkbox" onchange="checkSettingsModified()"> Полный экран
              </label>
            </div>
          </div>
        </div>

        <!-- Floating Save Changes Bar -->
        <div id="settings-save-bar" class="settings-save-bar">
          <div class="settings-save-info">
            <span class="settings-save-dot"></span>
            <span>Параметры изменены</span>
          </div>
          <div style="display: flex; gap: 8px;">
            <button class="btn" onclick="resetSettingsInputs()">Сбросить</button>
            <button class="btn btn-primary" onclick="saveSettings()">Сохранить изменения</button>
          </div>
        </div>
      </div>

    </div>
  </div>

  <!-- MODAL: ACCOUNTS & MICROSOFT AUTH -->
  <div id="modal-accounts" class="modal-overlay">
    <div class="modal-box">
      <div class="modal-title">Аккаунты Minecraft</div>
      
      <div id="accounts-list-box" style="display: flex; flex-direction: column; gap: 8px; max-height: 200px; overflow-y: auto;">
        <!-- Injected by JS -->
      </div>

      <div style="border-top: 1px solid rgba(255,255,255,0.08); padding-top: 14px;">
        <div style="font-size: 13px; font-weight: 600; margin-bottom: 8px;">Добавить аккаунт</div>
        <div style="display: flex; gap: 8px; margin-bottom: 12px;">
          <input id="new-offline-name" type="text" class="form-input" placeholder="Никнейм игрока" style="flex: 1;" onkeydown="if(event.key==='Enter')addOfflineAccount()">
          <button class="btn btn-primary" onclick="addOfflineAccount()">Создать Offline</button>
        </div>
        <button class="btn" style="width: 100%; justify-content: center; gap: 10px;" onclick="startMicrosoftLogin()">
          <span>&#10037;</span>
          <span>Войти через Microsoft (Xbox Live)</span>
        </button>
      </div>

      <!-- MS Auth Window Prompt -->
      <div id="ms-auth-box" style="display: none; background: rgba(99, 102, 241, 0.1); border: 1px solid rgba(99, 102, 241, 0.3); border-radius: var(--radius-sm); padding: 16px; text-align: center; margin-top: 10px;">
        <div style="font-size: 14px; font-weight: 600; color: #f1f5f9; margin-bottom: 6px;">Вход через Microsoft</div>
        <div id="ms-status-msg" style="font-size: 12.5px; color: var(--text-secondary); margin-bottom: 8px;">Открыто окно авторизации Microsoft...</div>
      </div>

      <div class="modal-actions">
        <button class="btn" onclick="closeAccountsModal()">Закрыть</button>
      </div>
    </div>
  </div>

  <!-- MODAL: CREATE INSTANCE -->
  <div id="modal-create-inst" class="modal-overlay">
    <div class="modal-box">
      <div class="modal-title">Создать новую сборку</div>
      
      <div class="form-group">
        <label class="form-label">Название сборки</label>
        <input id="create-inst-name" type="text" class="form-input" placeholder="Моя сборка 1.21.1" onkeydown="if(event.key==='Enter')confirmCreateInstance()">
      </div>

      <div class="form-group">
        <label class="form-label">Версия Minecraft</label>
        <select id="create-inst-ver" class="form-select">
          <option value="1.21.4">1.21.4 (Новейшая)</option>
          <option value="1.21.1" selected>1.21.1 (Рекомендуется)</option>
          <option value="1.20.4">1.20.4</option>
          <option value="1.20.1">1.20.1 (Стабильная)</option>
          <option value="1.19.4">1.19.4</option>
          <option value="1.18.2">1.18.2</option>
          <option value="1.16.5">1.16.5 (Классика)</option>
          <option value="1.12.2">1.12.2</option>
        </select>
      </div>

      <div class="form-group">
        <label class="form-label">Загрузчик модов</label>
        <select id="create-inst-loader" class="form-select">
          <option value="fabric">Fabric (Рекомендуется)</option>
          <option value="vanilla">Vanilla (Чистый Minecraft)</option>
          <option value="forge">Forge</option>
          <option value="neoforge">NeoForge</option>
        </select>
      </div>

      <div class="modal-actions">
        <button class="btn" onclick="closeCreateInstanceModal()">Отмена</button>
        <button class="btn btn-primary" onclick="confirmCreateInstance()">Создать сборку</button>
      </div>
    </div>
  </div>

  <!-- MODAL: EDIT INSTANCE & SETTINGS -->
  <div id="modal-edit-inst" class="modal-overlay">
    <div class="modal-box" style="max-width: 680px; width: 95%; max-height: 90vh; display: flex; flex-direction: column;">
      <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 14px;">
        <div>
          <div class="modal-title" style="margin-bottom: 2px;">Настройки сборки</div>
          <div id="edit-inst-subtitle" style="font-size: 13px; color: var(--text-secondary);">Управление версией, модами и параметрами Java</div>
        </div>
        <button class="win-action-btn btn-close" onclick="closeEditInstanceModal()" style="font-size: 14px; width: 28px; height: 28px;">&#x2715;</button>
      </div>

      <!-- Navigation tabs in modal -->
      <div style="display: flex; gap: 8px; border-bottom: 1px solid var(--surface-glass-border); padding-bottom: 10px; margin-bottom: 14px;">
        <button id="tab-btn-inst-gen" class="btn btn-primary" onclick="switchInstTab('general')">&#9881; Параметры</button>
        <button id="tab-btn-inst-mods" class="btn" onclick="switchInstTab('mods')">&#128230; Моды (<span id="edit-inst-mods-count">0</span>)</button>
        <button id="tab-btn-inst-perf" class="btn" onclick="switchInstTab('perf')">&#9889; Память и Java</button>
      </div>

      <!-- Tab: General -->
      <div id="inst-tab-general" style="overflow-y: auto; flex: 1; padding-right: 4px;">
        <div class="form-group">
          <label class="form-label">Название сборки</label>
          <input id="edit-inst-name" type="text" class="form-input">
        </div>
        <div class="form-group">
          <label class="form-label">Версия Minecraft</label>
          <select id="edit-inst-ver" class="form-select" onchange="onEditInstVerChange()">
            <option value="1.21.4">1.21.4 (Новейшая)</option>
            <option value="1.21.1">1.21.1</option>
            <option value="1.20.4">1.20.4</option>
            <option value="1.20.1">1.20.1</option>
            <option value="1.19.4">1.19.4</option>
            <option value="1.18.2">1.18.2</option>
            <option value="1.16.5">1.16.5</option>
            <option value="1.12.2">1.12.2</option>
          </select>
        </div>
        <div class="form-group">
          <label class="form-label">Загрузчик модов</label>
          <select id="edit-inst-loader" class="form-select" onchange="onEditInstLoaderChange()">
            <option value="vanilla">Vanilla (Оригинальная игра без модов)</option>
            <option value="fabric">Fabric (Быстрый современный загрузчик)</option>
            <option value="forge">Forge</option>
            <option value="neoforge">NeoForge</option>
          </select>
        </div>
        <div id="edit-inst-fabric-group" class="form-group">
          <label class="form-label">Версия Fabric Loader</label>
          <select id="edit-inst-fabric-ver" class="form-select">
            <option value="">Автоматически (последняя стабильная версия)</option>
          </select>
        </div>
      </div>

      <!-- Tab: Mods -->
      <div id="inst-tab-mods" style="display: none; overflow-y: auto; flex: 1; padding-right: 4px;">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; gap: 8px;">
          <div style="display: flex; gap: 8px;">
            <button class="btn btn-primary" onclick="addModsDialog()">+ Добавить моды (.jar)</button>
            <button class="btn" onclick="openCurrentModsFolder()">&#128194; Папка модов</button>
          </div>
          <button class="btn" onclick="refreshModsList()">&#8635; Обновить</button>
        </div>
        <div id="edit-inst-mods-list" style="display: flex; flex-direction: column; gap: 6px; min-height: 200px; max-height: 320px; overflow-y: auto; background: rgba(0,0,0,0.25); border-radius: var(--radius-sm); padding: 8px; border: 1px solid var(--surface-glass-border);">
          <!-- Injected by JS -->
        </div>
      </div>

      <!-- Tab: Java & Performance -->
      <div id="inst-tab-perf" style="display: none; overflow-y: auto; flex: 1; padding-right: 4px;">
        <div class="form-group">
          <div style="display: flex; justify-content: space-between; margin-bottom: 8px;">
            <label class="form-label">Выделение оперативной памяти (RAM)</label>
            <span id="edit-inst-ram-val" style="font-size: 13px; font-weight: 600; color: var(--accent);">4096 MB</span>
          </div>
          <input id="edit-inst-ram" type="range" min="1024" max="16384" step="512" style="width: 100%; accent-color: var(--primary);" oninput="document.getElementById('edit-inst-ram-val').innerText = this.value + ' MB (' + (this.value/1024).toFixed(1) + ' GB)'">
        </div>
        <div class="form-group">
          <label class="form-label">Выбор Java для этой сборки</label>
          <select id="edit-inst-java" class="form-select">
            <option value="">Автоматически (рекомендуемая для версии)</option>
          </select>
        </div>
        <div class="form-group">
          <label class="form-label">Дополнительные аргументы JVM</label>
          <input id="edit-inst-jvm" type="text" class="form-input" placeholder="-XX:+UseG1GC ...">
        </div>
      </div>

      <div class="modal-actions" style="margin-top: 16px; border-top: 1px solid var(--surface-glass-border); padding-top: 12px;">
        <button class="btn" onclick="closeEditInstanceModal()">Отмена</button>
        <button class="btn btn-primary" onclick="confirmSaveInstanceConfig()">Сохранить настройки</button>
      </div>
    </div>
  </div>

  <div id="toast-container" class="toast-container"></div>

  <script>
    // Drag window support
    const titlebar = document.getElementById('drag-titlebar');
    titlebar.addEventListener('mousedown', (e) => {
      if (e.target.closest('.titlebar-actions')) return;
      callNative('window_drag', {});
    });

    // State
    let currentTab = 'home';
    let appData = {
      instances: [],
      accounts: [],
      settings: {},
      versions: [],
      javaList: []
    };
    let msPollTimer = null;

    // Toast helper
    function showToast(msg) {
      const container = document.getElementById('toast-container');
      const toast = document.createElement('div');
      toast.className = 'toast';
      toast.innerHTML = `<span>&#9432;</span> <span>${msg}</span>`;
      container.appendChild(toast);
      setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transition = 'opacity 0.3s ease';
        setTimeout(() => toast.remove(), 300);
      }, 3500);
    }

    // Tab switcher
    function switchTab(tabId) {
      if (currentTab === 'settings' && tabId !== 'settings') {
        resetSettingsInputs();
      }
      const bar = document.getElementById('settings-save-bar');
      if (bar) bar.classList.remove('visible');

      currentTab = tabId;
      document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
      document.querySelectorAll('.nav-item').forEach(el => el.classList.remove('active'));

      const target = document.getElementById('tab-' + tabId);
      if (target) target.classList.add('active');

      const navs = document.querySelectorAll('.nav-item');
      if (tabId === 'home') navs[0].classList.add('active');
      else if (tabId === 'instances') navs[1].classList.add('active');
      else if (tabId === 'modpacks') navs[2].classList.add('active');
      else if (tabId === 'mods') navs[3].classList.add('active');
      else if (tabId === 'versions') navs[4].classList.add('active');
      else if (tabId === 'settings') {
        navs[5].classList.add('active');
        checkSettingsModified();
      }

      if (tabId === 'modpacks' && (!window.modpacksLoaded)) {
        searchModpacks();
        window.modpacksLoaded = true;
      } else if (tabId === 'mods' && (!window.modsLoaded)) {
        searchMods();
        window.modsLoaded = true;
      } else if (tabId === 'versions' && (!window.versionsLoaded)) {
        loadVersions();
        window.versionsLoaded = true;
      }
    }

    // Call native bridge
    function callNative(action, payload) {
      const data = JSON.stringify({ action: action, payload: payload || {} });
      if (typeof window.callNativeBridge === 'function') {
        window.callNativeBridge(data);
      } else {
        const timer = setInterval(() => {
          if (typeof window.callNativeBridge === 'function') {
            clearInterval(timer);
            window.callNativeBridge(data);
          }
        }, 20);
        setTimeout(() => clearInterval(timer), 3000);
      }
    }

    // Receiver for messages from native app
    window.onNativeMessage = function(data) {
      const msg = typeof data === 'string' ? JSON.parse(data) : data;
      const type = msg.type;
      const payload = msg.payload;

      if (type === 'init_data') {
        appData.instances = payload.instances || [];
        appData.accounts = payload.accounts || [];
        appData.settings = payload.settings || {};
        appData.javaList = payload.javaList || [];
        appData.system_ram_mb = payload.system_ram_mb || 8192;
        appData.system_res_w = payload.system_res_w || 1920;
        appData.system_res_h = payload.system_res_h || 1080;
        renderUI();
      } else if (type === 'java_selected') {
        const p = payload.path;
        if (p) {
          const javaSelect = document.getElementById('setting-java');
          let opt = Array.from(javaSelect.options).find(o => o.value === p);
          if (!opt) {
            opt = document.createElement('option');
            opt.value = p;
            opt.innerText = `Выбранный файл (${p})`;
            javaSelect.appendChild(opt);
          }
          opt.selected = true;
          showToast('Выбран Java: ' + p);
          checkSettingsModified();
        }
      } else if (type === 'launch_status') {
        updateLaunchStatus(payload);
      } else if (type === 'search_results') {
        if (payload.target === 'modpacks') renderModpacks(payload.results);
        else if (payload.target === 'mods') renderMods(payload.results);
      } else if (type === 'versions_list') {
        renderVersions(payload);
      } else if (type === 'ms_login_started') {
        const msg = document.getElementById('ms-status-msg');
        if (msg) msg.innerText = 'Выполните вход в открывшемся окне Microsoft...';
      } else if (type === 'ms_auth_result') {
        handleMsAuthResult(payload);
      } else if (type === 'instance_details') {
        handleInstanceDetails(payload);
      } else if (type === 'fabric_loaders_result') {
        handleFabricLoadersResult(payload);
      } else if (type === 'toast') {
        showToast(payload.message);
      }
    };

    function renderUI() {
      renderActiveInstanceHero();
      renderInstancesList();
      renderQuickInstances();
      renderActiveAccount();
      renderSettings();
      renderAccountsModalList();
    }

    function renderActiveInstanceHero() {
      const inst = appData.instances.find(i => i.is_active) || appData.instances[0];
      const btnPlay = document.getElementById('btn-play');
      if (!inst) {
        document.getElementById('hero-title').innerText = 'Нет сборок';
        document.getElementById('hero-tag-ver').innerText = '-';
        document.getElementById('hero-tag-loader').innerText = '-';
        document.getElementById('hero-desc').innerText = 'Создайте новую сборку или выберите модпак из каталога.';
        if (btnPlay) btnPlay.disabled = true;
        return;
      }

      if (btnPlay) btnPlay.disabled = false;
      document.getElementById('hero-title').innerText = inst.name;
      document.getElementById('hero-tag-ver').innerText = 'MC ' + inst.game_version;
      document.getElementById('hero-tag-loader').innerText = inst.loader_type.toUpperCase();
      document.getElementById('hero-desc').innerText = `Сборка на ${inst.loader_type.toUpperCase()} с ${inst.mod_count || 0} установленными модами.`;
    }

    function renderQuickInstances() {
      const c = document.getElementById('quick-instances');
      c.innerHTML = '';
      if (!appData.instances || appData.instances.length === 0) {
        c.innerHTML = '<div style="color: var(--text-muted); font-size: 13px; padding: 12px;">Сборки еще не созданы. Добавьте сборку во вкладке «Мои сборки» или каталоге.</div>';
        return;
      }
      appData.instances.slice(0, 3).forEach(inst => {
        const card = document.createElement('div');
        card.className = 'card ' + (inst.is_active ? 'active' : '');
        card.innerHTML = `
          <div class="card-top">
            <div class="card-icon">&#9881;</div>
            <div class="card-meta">
              <div class="card-title">${inst.name}</div>
              <div class="card-subtitle">Версия: ${inst.game_version} (${inst.loader_type.toUpperCase()})</div>
            </div>
          </div>
          <div class="card-actions">
            <span style="font-size: 12px; color: var(--text-muted);">${inst.mod_count || 0} модов</span>
            <div style="display: flex; gap: 6px;">
              <button class="btn" onclick="openEditInstanceModal('${inst.id}')" title="Настройки сборки">&#9881; Настроить</button>
              <button class="btn btn-primary" onclick="selectInstance('${inst.id}')">Выбрать</button>
            </div>
          </div>
        `;
        c.appendChild(card);
      });
    }

    function renderInstancesList() {
      const c = document.getElementById('instances-list');
      c.innerHTML = '';
      if (!appData.instances || appData.instances.length === 0) {
        c.innerHTML = '<div style="color: var(--text-muted); font-size: 14px; padding: 20px;">Нет созданных сборок. Нажмите кнопку «+ Создать сборку» выше или перейдите в «Каталог сборок».</div>';
        return;
      }
      appData.instances.forEach(inst => {
        const card = document.createElement('div');
        card.className = 'card ' + (inst.is_active ? 'active' : '');
        card.innerHTML = `
          <div class="card-top">
            <div class="card-icon">&#128230;</div>
            <div class="card-meta">
              <div class="card-title">${inst.name}</div>
              <div class="card-subtitle">${inst.game_version} • ${inst.loader_type.toUpperCase()}</div>
            </div>
          </div>
          <div class="card-desc">
            Память: ${inst.ram_max_mb} MB | Модов: ${inst.mod_count || 0}
          </div>
          <div class="card-actions">
            <button class="btn btn-danger" onclick="deleteInstance('${inst.id}')">Удалить</button>
            <div style="display: flex; gap: 8px;">
              <button class="btn" onclick="openEditInstanceModal('${inst.id}')">&#9881; Настроить</button>
              <button class="btn" onclick="openInstanceFolder('${inst.id}')">&#128194; Папка</button>
              <button class="btn ${inst.is_active ? 'btn-primary' : ''}" onclick="selectInstance('${inst.id}')">
                ${inst.is_active ? 'Выбрана' : 'Выбрать'}
              </button>
            </div>
          </div>
        `;
        c.appendChild(card);
      });
    }

    function renderActiveAccount() {
      const acc = appData.accounts.find(a => a.is_active) || appData.accounts[0];
      if (!acc) return;

      document.getElementById('user-name').innerText = acc.username;
      document.getElementById('user-type-text').innerText = acc.type === 'microsoft' ? 'Microsoft' : 'Offline';
      const img = document.getElementById('user-avatar');
      img.src = acc.skin_url || `https://minotar.net/helm/${encodeURIComponent(acc.username)}/100.png`;
      img.onerror = function() {
        this.src = 'https://minotar.net/helm/char/100.png';
      };
    }

    function renderSettings() {
      const s = appData.settings;
      if (!s) return;

      const totalRam = appData.system_ram_mb || 16384;
      const slider = document.getElementById('setting-ram');
      slider.max = totalRam;
      const ramVal = s.ram_max_mb || 4096;
      slider.value = ramVal;
      updateRamLabel(ramVal);

      document.getElementById('setting-jvm').value = s.jvm_args || '';
      document.getElementById('setting-res-w').value = s.resolution_w || (appData.system_res_w || 1280);
      document.getElementById('setting-res-h').value = s.resolution_h || (appData.system_res_h || 720);
      document.getElementById('setting-fullscreen').checked = !!s.fullscreen;

      const javaSelect = document.getElementById('setting-java');
      javaSelect.innerHTML = '';

      const defOpt = document.createElement('option');
      defOpt.value = 'javaw.exe';
      defOpt.innerText = 'Автоматически из системы (javaw.exe)';
      if (!s.java_path || s.java_path === 'javaw.exe') defOpt.selected = true;
      javaSelect.appendChild(defOpt);

      let found = (!s.java_path || s.java_path === 'javaw.exe');
      (appData.javaList || []).forEach(j => {
        const opt = document.createElement('option');
        opt.value = j.path;
        opt.innerText = `${j.version} (${j.path})`;
        if (j.path === s.java_path) {
          opt.selected = true;
          found = true;
        }
        javaSelect.appendChild(opt);
      });

      if (!found && s.java_path) {
        const customOpt = document.createElement('option');
        customOpt.value = s.java_path;
        customOpt.innerText = `Пользовательский путь (${s.java_path})`;
        customOpt.selected = true;
        javaSelect.appendChild(customOpt);
      }

      checkSettingsModified();
    }

    function checkSettingsModified() {
      if (!appData.settings) return false;
      const s = appData.settings;
      const ramEl = document.getElementById('setting-ram');
      const javaEl = document.getElementById('setting-java');
      const jvmEl = document.getElementById('setting-jvm');
      const rwEl = document.getElementById('setting-res-w');
      const rhEl = document.getElementById('setting-res-h');
      const fullEl = document.getElementById('setting-fullscreen');
      if (!ramEl || !javaEl || !jvmEl || !rwEl || !rhEl || !fullEl) return false;

      const curRam = parseInt(ramEl.value) || 4096;
      const curJava = javaEl.value || 'javaw.exe';
      const curJvm = (jvmEl.value || '').trim();
      const curW = parseInt(rwEl.value) || (appData.system_res_w || 1280);
      const curH = parseInt(rhEl.value) || (appData.system_res_h || 720);
      const curFull = fullEl.checked;

      const savedRam = s.ram_max_mb || 4096;
      const savedJava = s.java_path || 'javaw.exe';
      const savedJvm = (s.jvm_args || '').trim();
      const savedW = s.resolution_w || (appData.system_res_w || 1280);
      const savedH = s.resolution_h || (appData.system_res_h || 720);
      const savedFull = !!s.fullscreen;

      const isModified = (curRam !== savedRam ||
                          curJava !== savedJava ||
                          curJvm !== savedJvm ||
                          curW !== savedW ||
                          curH !== savedH ||
                          curFull !== savedFull);

      const bar = document.getElementById('settings-save-bar');
      if (bar) {
        if (isModified && currentTab === 'settings') {
          bar.classList.add('visible');
        } else {
          bar.classList.remove('visible');
        }
      }
      return isModified;
    }

    function resetSettingsInputs() {
      renderSettings();
      const bar = document.getElementById('settings-save-bar');
      if (bar) bar.classList.remove('visible');
    }

    function updateRamLabel(val) {
      document.getElementById('ram-val').innerText = val + ' MB';
      const sub = document.getElementById('ram-subtext');
      if (sub && appData.system_ram_mb) {
        sub.innerText = `Выделено ${val} МБ (${(val/1024).toFixed(1)} ГБ) из ${appData.system_ram_mb} МБ (${Math.round(appData.system_ram_mb/1024)} ГБ) доступных в системе`;
      }
    }

    function setResPreset(w, h) {
      document.getElementById('setting-res-w').value = w;
      document.getElementById('setting-res-h').value = h;
      checkSettingsModified();
    }

    function setNativeRes() {
      if (appData.system_res_w && appData.system_res_h) {
        setResPreset(appData.system_res_w, appData.system_res_h);
      }
    }

    function browseJava() {
      callNative('browse_java', {});
    }

    function saveSettings() {
      const s = {
        ram_max_mb: parseInt(document.getElementById('setting-ram').value),
        java_path: document.getElementById('setting-java').value,
        jvm_args: document.getElementById('setting-jvm').value,
        resolution_w: parseInt(document.getElementById('setting-res-w').value),
        resolution_h: parseInt(document.getElementById('setting-res-h').value),
        fullscreen: document.getElementById('setting-fullscreen').checked
      };
      appData.settings = Object.assign({}, appData.settings, s);
      callNative('save_settings', s);
      const bar = document.getElementById('settings-save-bar');
      if (bar) bar.classList.remove('visible');
    }

    // Launch Game
    function launchActiveInstance() {
      const inst = appData.instances.find(i => i.is_active) || appData.instances[0];
      if (!inst) {
        showToast('Сначала создайте сборку или выберите модпак из каталога!');
        return;
      }
      const btn = document.getElementById('btn-play');
      btn.disabled = true;
      document.getElementById('launch-progress').style.display = 'block';
      callNative('launch_game', {});
    }

    function updateLaunchStatus(status) {
      const fill = document.getElementById('progress-fill');
      const text = document.getElementById('progress-status-text');
      const pct = document.getElementById('progress-percent-text');
      const btn = document.getElementById('btn-play');

      fill.style.width = status.progress + '%';
      pct.innerText = status.progress + '%';
      text.innerText = status.details || status.stage;

      if (status.is_running) {
        btn.disabled = true;
        btn.innerHTML = `<span>&#9642;</span> <span>ИГРАЕТ</span>`;
      } else if (status.stage === 'idle') {
        btn.disabled = false;
        btn.innerHTML = `<span>&#9658;</span> <span>ИГРАТЬ</span>`;
        document.getElementById('launch-progress').style.display = 'none';
      } else if (status.stage === 'error') {
        btn.disabled = false;
        btn.innerHTML = `<span>&#9658;</span> <span>ИГРАТЬ</span>`;
        document.getElementById('launch-progress').style.display = 'none';
        const msg = status.details || status.error_message || 'Неизвестная ошибка запуска';
        showToast('Ошибка запуска: ' + msg);
        alert('Ошибка запуска Minecraft:\n' + msg);
      }
    }

    // Instances methods
    function selectInstance(id) {
      callNative('select_instance', { id: id });
    }

    function deleteInstance(id) {
      if (confirm('Удалить эту сборку?')) {
        callNative('delete_instance', { id: id });
      }
    }

    function openInstanceFolder(id) {
      callNative('open_instance_folder', { id: id });
    }

    function openCreateInstanceModal() {
      document.getElementById('modal-create-inst').style.display = 'flex';
    }
    function closeCreateInstanceModal() {
      document.getElementById('modal-create-inst').style.display = 'none';
    }
    function confirmCreateInstance() {
      const name = document.getElementById('create-inst-name').value.trim();
      const ver = document.getElementById('create-inst-ver').value;
      const loader = document.getElementById('create-inst-loader').value;
      if (!name) {
        showToast('Введите название сборки!');
        return;
      }
      callNative('create_instance', { name: name, game_version: ver, loader_type: loader });
      closeCreateInstanceModal();
    }

    // Instance Settings & Mods
    let currentEditInstanceId = '';

    function openActiveInstanceSettings() {
      const active = appData.instances.find(i => i.is_active) || appData.instances[0];
      if (active) openEditInstanceModal(active.id);
    }

    function openEditInstanceModal(id) {
      currentEditInstanceId = id;
      const modal = document.getElementById('modal-edit-inst');
      modal.style.display = 'flex';
      switchInstTab('general');
      
      const inst = appData.instances.find(i => i.id === id);
      if (inst) {
        document.getElementById('edit-inst-name').value = inst.name || '';
        document.getElementById('edit-inst-ver').value = inst.game_version || '1.21.1';
        document.getElementById('edit-inst-loader').value = inst.loader_type || 'vanilla';
        document.getElementById('edit-inst-subtitle').innerText = `Сборка: ${inst.name} (${inst.game_version})`;
        
        const totalRam = appData.system_ram_mb || 16384;
        const ramInput = document.getElementById('edit-inst-ram');
        ramInput.max = totalRam;
        const ramVal = inst.ram_max_mb || (appData.settings ? appData.settings.ram_max_mb : 4096);
        ramInput.value = ramVal;
        document.getElementById('edit-inst-ram-val').innerText = ramVal + ' MB (' + (ramVal/1024).toFixed(1) + ' GB)';
        
        document.getElementById('edit-inst-jvm').value = inst.jvm_args || '';
        
        const javaSel = document.getElementById('edit-inst-java');
        javaSel.innerHTML = '<option value="">Автоматически (рекомендуемая для версии)</option>';
        (appData.javaList || []).forEach(j => {
          const opt = document.createElement('option');
          opt.value = j.path;
          opt.innerText = `${j.version} (${j.path})`;
          if (inst.java_path === j.path) opt.selected = true;
          javaSel.appendChild(opt);
        });
        if (inst.java_path && !Array.from(javaSel.options).some(o => o.value === inst.java_path)) {
          const opt = document.createElement('option');
          opt.value = inst.java_path;
          opt.innerText = `Пользовательский (${inst.java_path})`;
          opt.selected = true;
          javaSel.appendChild(opt);
        }

        onEditInstLoaderChange();
      }

      callNative('get_instance_details', { id: id });
    }

    function closeEditInstanceModal() {
      document.getElementById('modal-edit-inst').style.display = 'none';
      currentEditInstanceId = '';
    }

    function switchInstTab(tab) {
      document.getElementById('inst-tab-general').style.display = (tab === 'general' ? 'block' : 'none');
      document.getElementById('inst-tab-mods').style.display = (tab === 'mods' ? 'block' : 'none');
      document.getElementById('inst-tab-perf').style.display = (tab === 'perf' ? 'block' : 'none');

      document.getElementById('tab-btn-inst-gen').className = 'btn ' + (tab === 'general' ? 'btn-primary' : '');
      document.getElementById('tab-btn-inst-mods').className = 'btn ' + (tab === 'mods' ? 'btn-primary' : '');
      document.getElementById('tab-btn-inst-perf').className = 'btn ' + (tab === 'perf' ? 'btn-primary' : '');
    }

    function onEditInstLoaderChange() {
      const loader = document.getElementById('edit-inst-loader').value;
      const grp = document.getElementById('edit-inst-fabric-group');
      if (loader === 'fabric') {
        grp.style.display = 'block';
      } else {
        grp.style.display = 'none';
      }
    }

    function onEditInstVerChange() {
      const ver = document.getElementById('edit-inst-ver').value;
      callNative('get_fabric_loaders', { game_version: ver });
    }

    function handleInstanceDetails(p) {
      if (!currentEditInstanceId || p.id !== currentEditInstanceId) return;

      document.getElementById('edit-inst-name').value = p.name || '';
      document.getElementById('edit-inst-ver').value = p.game_version || '1.21.1';
      document.getElementById('edit-inst-loader').value = p.loader_type || 'vanilla';
      document.getElementById('edit-inst-subtitle').innerText = `Сборка: ${p.name} (${p.game_version})`;
      
      onEditInstLoaderChange();

      const fabSel = document.getElementById('edit-inst-fabric-ver');
      fabSel.innerHTML = '<option value="">Автоматически (последняя стабильная версия)</option>';
      if (p.fabric_loaders && Array.isArray(p.fabric_loaders)) {
        p.fabric_loaders.forEach(ver => {
          const opt = document.createElement('option');
          opt.value = ver;
          opt.innerText = ver;
          if (p.loader_version === ver) opt.selected = true;
          fabSel.appendChild(opt);
        });
      }

      renderEditInstMods(p.mods || []);
    }

    function handleFabricLoadersResult(p) {
      const fabSel = document.getElementById('edit-inst-fabric-ver');
      if (!fabSel) return;
      const cur = fabSel.value;
      fabSel.innerHTML = '<option value="">Автоматически (последняя стабильная версия)</option>';
      if (p.loaders && Array.isArray(p.loaders)) {
        p.loaders.forEach(ver => {
          const opt = document.createElement('option');
          opt.value = ver;
          opt.innerText = ver;
          if (cur === ver) opt.selected = true;
          fabSel.appendChild(opt);
        });
      }
    }

    function renderEditInstMods(mods) {
      const c = document.getElementById('edit-inst-mods-list');
      const countEl = document.getElementById('edit-inst-mods-count');
      if (countEl) countEl.innerText = mods.length;

      if (!mods || mods.length === 0) {
        c.innerHTML = `
          <div style="text-align: center; padding: 30px; color: var(--text-muted); font-size: 13px;">
            В этой сборке пока нет файлов модов.<br>
            Нажмите «+ Добавить моды (.jar)» или перейдите на вкладку «Моды» для установки в один клик.
          </div>
        `;
        return;
      }

      c.innerHTML = '';
      mods.forEach(m => {
        const row = document.createElement('div');
        row.style = 'display: flex; align-items: center; justify-content: space-between; padding: 8px 12px; background: var(--surface-card); border-radius: var(--radius-sm); border: 1px solid var(--surface-glass-border); gap: 10px;';
        
        const sizeMb = m.size ? (m.size / (1024 * 1024)).toFixed(2) + ' МБ' : '';

        row.innerHTML = `
          <div style="display: flex; align-items: center; gap: 10px; overflow: hidden; flex: 1;">
            <input type="checkbox" ${m.is_enabled ? 'checked' : ''} onchange="toggleMod('${m.filename}')" style="cursor: pointer; width: 16px; height: 16px; accent-color: var(--primary);">
            <div style="overflow: hidden; text-overflow: ellipsis; white-space: nowrap;">
              <div style="font-size: 13px; font-weight: 500; color: ${m.is_enabled ? 'var(--text-main)' : 'var(--text-muted)'}; text-decoration: ${m.is_enabled ? 'none' : 'line-through'};">
                ${m.filename}
              </div>
              <div style="font-size: 11px; color: var(--text-muted);">${sizeMb} ${m.is_enabled ? '• Активен' : '• Отключен'}</div>
            </div>
          </div>
          <button class="win-action-btn btn-close" title="Удалить мод" onclick="deleteMod('${m.filename}')" style="opacity: 0.8; font-size: 14px; width: 26px; height: 26px;">&#x2715;</button>
        `;
        c.appendChild(row);
      });
    }

    function toggleMod(filename) {
      if (!currentEditInstanceId) return;
      callNative('toggle_instance_mod', { id: currentEditInstanceId, filename: filename });
    }

    function deleteMod(filename) {
      if (!currentEditInstanceId) return;
      if (confirm("Удалить файл мода '" + filename + "'?")) {
        callNative('delete_instance_mod', { id: currentEditInstanceId, filename: filename });
      }
    }

    function addModsDialog() {
      if (!currentEditInstanceId) return;
      callNative('add_mods_dialog', { id: currentEditInstanceId });
    }

    function openCurrentModsFolder() {
      if (!currentEditInstanceId) return;
      callNative('open_mods_folder', { id: currentEditInstanceId });
    }

    function refreshModsList() {
      if (!currentEditInstanceId) return;
      callNative('get_instance_details', { id: currentEditInstanceId });
    }

    function confirmSaveInstanceConfig() {
      if (!currentEditInstanceId) return;
      const payload = {
        id: currentEditInstanceId,
        name: document.getElementById('edit-inst-name').value.trim(),
        game_version: document.getElementById('edit-inst-ver').value,
        loader_type: document.getElementById('edit-inst-loader').value,
        loader_version: document.getElementById('edit-inst-fabric-ver').value,
        ram_max_mb: parseInt(document.getElementById('edit-inst-ram').value),
        java_path: document.getElementById('edit-inst-java').value,
        jvm_args: document.getElementById('edit-inst-jvm').value.trim()
      };
      if (!payload.name) {
        showToast('Имя сборки не может быть пустым!');
        return;
      }
      callNative('save_instance_config', payload);
      closeEditInstanceModal();
    }

    // Modpacks
    function searchModpacks() {
      const q = document.getElementById('modpack-search').value;
      callNative('search_modpacks', { query: q });
    }

    let currentModpacks = [];
    function renderModpacks(packs) {
      currentModpacks = packs || [];
      const c = document.getElementById('modpacks-list');
      c.innerHTML = '';
      if (!packs || packs.length === 0) {
        c.innerHTML = '<div style="color: var(--text-muted); padding: 20px;">Ничего не найдено</div>';
        return;
      }
      packs.forEach((p, idx) => {
        const card = document.createElement('div');
        card.className = 'card';
        card.innerHTML = `
          <div class="card-top">
            <img class="card-icon" src="${p.icon_url || 'https://api.iconify.design/pixelarticons:cube.svg'}" onerror="this.src='https://api.iconify.design/pixelarticons:cube.svg'">
            <div class="card-meta">
              <div class="card-title">${p.title}</div>
              <div class="card-subtitle">Автор: ${p.author} • ${p.downloads.toLocaleString()} скачиваний</div>
            </div>
          </div>
          <div class="card-desc">${p.description}</div>
          <div class="card-actions">
            <span style="font-size: 11px; color: var(--accent);">${p.categories.slice(0, 2).join(', ')}</span>
            <button class="btn btn-primary" onclick="installModpackByIndex(${idx})">Скачать сборку</button>
          </div>
        `;
        c.appendChild(card);
      });
    }

    function installModpackByIndex(idx) {
      const p = currentModpacks[idx];
      if (!p) return;
      callNative('install_modpack', { project_id: p.id, title: p.title });
      showToast('Установка сборки "' + p.title + '" начата...');
    }

    // Mods
    function searchMods() {
      const q = document.getElementById('mod-search').value;
      callNative('search_mods', { query: q });
    }

    let currentMods = [];
    function renderMods(mods) {
      currentMods = mods || [];
      const c = document.getElementById('mods-list');
      c.innerHTML = '';
      if (!mods || mods.length === 0) {
        c.innerHTML = '<div style="color: var(--text-muted); padding: 20px;">Ничего не найдено</div>';
        return;
      }
      mods.forEach((m, idx) => {
        const card = document.createElement('div');
        card.className = 'card';
        card.innerHTML = `
          <div class="card-top">
            <img class="card-icon" src="${m.icon_url || 'https://api.iconify.design/pixelarticons:gear.svg'}" onerror="this.src='https://api.iconify.design/pixelarticons:gear.svg'">
            <div class="card-meta">
              <div class="card-title">${m.title}</div>
              <div class="card-subtitle">Автор: ${m.author} • ${m.downloads.toLocaleString()} скачиваний</div>
            </div>
          </div>
          <div class="card-desc">${m.description}</div>
          <div class="card-actions">
            <span style="font-size: 11px; color: var(--accent);">${m.categories.slice(0, 2).join(', ')}</span>
            <button class="btn btn-primary" onclick="installModByIndex(${idx})">В сборку</button>
          </div>
        `;
        c.appendChild(card);
      });
    }

    function installModByIndex(idx) {
      const m = currentMods[idx];
      if (!m) return;
      callNative('install_mod', { project_id: m.id, title: m.title });
      showToast('Загрузка мода "' + m.title + '" в активную сборку...');
    }

    // Versions
    function loadVersions() {
      callNative('get_versions', {});
    }

    function renderVersions(versions) {
      const c = document.getElementById('versions-list');
      c.innerHTML = '';
      versions.slice(0, 24).forEach(v => {
        const card = document.createElement('div');
        card.className = 'card';
        card.innerHTML = `
          <div class="card-top">
            <div class="card-icon">&#128392;</div>
            <div class="card-meta">
              <div class="card-title">Minecraft ${v.id}</div>
              <div class="card-subtitle">Релиз</div>
            </div>
          </div>
          <div class="card-actions">
            <span style="font-size: 11px; color: var(--text-muted);">${v.release_time.slice(0, 10)}</span>
            <button class="btn btn-primary" onclick="createInstanceFromVersion('${v.id}')">Создать сборку</button>
          </div>
        `;
        c.appendChild(card);
      });
    }

    function createInstanceFromVersion(ver) {
      callNative('create_instance', { name: 'Minecraft ' + ver, game_version: ver, loader_type: 'vanilla' });
      showToast('Создан профиль версии ' + ver);
      switchTab('home');
    }

    // Accounts & MS Login
    function openAccountsModal() {
      renderAccountsModalList();
      document.getElementById('modal-accounts').style.display = 'flex';
    }
    function closeAccountsModal() {
      if (msPollTimer) clearInterval(msPollTimer);
      document.getElementById('ms-auth-box').style.display = 'none';
      document.getElementById('modal-accounts').style.display = 'none';
    }

    function renderAccountsModalList() {
      const c = document.getElementById('accounts-list-box');
      if (!c) return;
      c.innerHTML = '';
      if (!appData.accounts || appData.accounts.length === 0) {
        c.innerHTML = '<div style="color: var(--text-muted); font-size: 12px; padding: 10px;">Нет добавленных аккаунтов</div>';
        return;
      }
      appData.accounts.forEach(acc => {
        const item = document.createElement('div');
        item.className = 'user-profile-card';
        item.style.padding = '8px 12px';
        const avatarUrl = acc.skin_url || `https://minotar.net/helm/${encodeURIComponent(acc.username)}/100.png`;
        item.innerHTML = `
          <img class="avatar-img" src="${avatarUrl}" style="width: 28px; height: 28px; border-radius: 6px;" onerror="this.src='https://minotar.net/helm/char/100.png'">
          <div class="user-info">
            <div class="user-name" style="font-size: 12.5px;">${acc.username}</div>
            <div class="user-type" style="font-size: 10px;">${acc.type === 'microsoft' ? 'Microsoft' : 'Offline'}</div>
          </div>
          <div style="display: flex; gap: 6px;">
            <button class="btn ${acc.is_active ? 'btn-primary' : ''}" style="padding: 4px 10px; font-size: 11px;" onclick="selectAccount('${acc.id}')">
              ${acc.is_active ? 'Активен' : 'Выбрать'}
            </button>
            <button class="btn btn-danger" style="padding: 4px 8px; font-size: 11px;" onclick="deleteAccount('${acc.id}')">&times;</button>
          </div>
        `;
        c.appendChild(item);
      });
    }

    function selectAccount(id) {
      callNative('select_account', { id: id });
    }
    function deleteAccount(id) {
      callNative('delete_account', { id: id });
    }
    function addOfflineAccount() {
      const name = document.getElementById('new-offline-name').value.trim();
      if (!name) return;
      callNative('add_offline_account', { username: name });
      document.getElementById('new-offline-name').value = '';
    }

    function startMicrosoftLogin() {
      document.getElementById('ms-auth-box').style.display = 'block';
      document.getElementById('ms-status-msg').innerText = 'Открытие окна авторизации Microsoft...';
      callNative('start_ms_login', {});
    }

    function handleMsAuthResult(res) {
      if (res.status === 'success') {
        if (msPollTimer) clearInterval(msPollTimer);
        document.getElementById('ms-status-msg').innerText = 'Успешно! Вход выполнен.';
        showToast('Добро пожаловать, ' + (res.account ? res.account.username : '') + '!');
        setTimeout(() => closeAccountsModal(), 1200);
      } else if (res.status === 'cancelled') {
        if (msPollTimer) clearInterval(msPollTimer);
        document.getElementById('ms-auth-box').style.display = 'none';
        showToast('Вход Microsoft отменен.');
      } else if (res.status === 'error' || res.status === 'expired') {
        if (msPollTimer) clearInterval(msPollTimer);
        document.getElementById('ms-status-msg').innerText = 'Ошибка: ' + res.message;
        showToast('Ошибка входа Microsoft: ' + res.message);
      }
    }

    // Window controls
    function minWindow() { callNative('window_minimize', {}); }
    function closeWindow() { callNative('window_close', {}); }

    // Init call on page load with fallbacks
    function initLauncher() {
      callNative('request_init', {});
    }
    window.addEventListener('DOMContentLoaded', initLauncher);
    setTimeout(initLauncher, 100);
    setTimeout(initLauncher, 500);
  </script>
</body>
</html>
)rawhtml";

} // namespace UI
