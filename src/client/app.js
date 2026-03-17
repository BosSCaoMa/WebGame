const API_BASE = window.WEBGAME_API_BASE || "/api";

const fallbackCharacters = [
  { id: 2001, name: "先锋战士", quality: "BLUE", position: "WARRIOR" },
  { id: 2002, name: "守护坦克", quality: "BLUE", position: "TANK" },
  { id: 2003, name: "后排术士", quality: "PURPLE", position: "MAGE" },
  { id: 2004, name: "战地医师", quality: "PURPLE", position: "HEALER" },
  { id: 2005, name: "疾风刺客", quality: "ORANGE", position: "ASSASSIN" },
  { id: 2006, name: "铁壁统领", quality: "ORANGE", position: "TANK" },
];

const fallbackItemSamples = {
  items: [],
  equipments: [],
  setBonuses: [],
};

const state = {
  account: localStorage.getItem("wg_account") || "",
  token: localStorage.getItem("wg_token") || "",
  characters: fallbackCharacters,
  itemSamples: fallbackItemSamples,
  lastBattleLogs: [],
  lastDamageBoard: [],
  lastDamageSummary: null,
  lastDaily: null,
  lastLeaderboard: [],
  profile: null,
  activeView: "home",
  damageSideTab: "user",
  processExpanded: false,
};

const battleScenarios = [
  {
    id: "chapter_1",
    name: "第一章：边境巡逻",
    desc: "敌军为轻装突袭队，适合熟悉战斗节奏。",
    enemyTeam: [2001, 2003, 2004],
  },
  {
    id: "chapter_2",
    name: "第二章：山谷伏击",
    desc: "敌军双前排配置，建议提高我方破甲与续航。",
    enemyTeam: [2002, 2006, 2004],
  },
  {
    id: "chapter_3",
    name: "第三章：王城决战",
    desc: "敌军高爆发阵容，建议控制敌方速度位。",
    enemyTeam: [2005, 2006, 2003],
  },
];

const qualityClassMap = {
  GREEN: "q-green",
  BLUE: "q-blue",
  PURPLE: "q-purple",
  ORANGE: "q-orange",
};

const els = {
  serverStatus: document.getElementById("serverStatus"),
  accountBadge: document.getElementById("accountBadge"),
  loginPanel: document.getElementById("loginPanel"),
  gamePanel: document.getElementById("gamePanel"),
  topNav: document.getElementById("topNav"),
  homeView: document.getElementById("homeView"),
  rankingView: document.getElementById("rankingView"),
  battleView: document.getElementById("battleView"),
  resultPanel: document.getElementById("resultPanel"),
  battleResultModal: document.getElementById("battleResultModal"),
  battleResultModalCard: document.getElementById("battleResultModalCard"),
  closeResultModalBtn: document.getElementById("closeResultModalBtn"),
  modalResultTitle: document.getElementById("modalResultTitle"),
  modalResultHeadline: document.getElementById("modalResultHeadline"),
  toggleBattleProcessBtn: document.getElementById("toggleBattleProcessBtn"),
  battleProcessSection: document.getElementById("battleProcessSection"),
  damageSideTabs: document.getElementById("damageSideTabs"),
  damageBoardList: document.getElementById("damageBoardList"),
  loginForm: document.getElementById("loginForm"),
  accountInput: document.getElementById("accountInput"),
  tokenInput: document.getElementById("tokenInput"),
  logoutBtn: document.getElementById("logoutBtn"),
  runAutoBtn: document.getElementById("runAutoBtn"),
  userTeamSlots: document.getElementById("userTeamSlots"),
  enemyTeamSlots: document.getElementById("enemyTeamSlots"),
  userItemSelect: document.getElementById("userItemSelect"),
  enemyItemSelect: document.getElementById("enemyItemSelect"),
  userEquipSelect: document.getElementById("userEquipSelect"),
  enemyEquipSelect: document.getElementById("enemyEquipSelect"),
  scenarioSelect: document.getElementById("scenarioSelect"),
  scenarioDesc: document.getElementById("scenarioDesc"),
  setBonusPreview: document.getElementById("setBonusPreview"),
  userTeamPreview: document.getElementById("userTeamPreview"),
  enemyTeamPreview: document.getElementById("enemyTeamPreview"),
  kpiGrid: document.getElementById("kpiGrid"),
  dataPreview: document.getElementById("dataPreview"),
  resultBadge: document.getElementById("resultBadge"),
  resultHeadline: document.getElementById("resultHeadline"),
  round: document.getElementById("round"),
  userTotalDamage: document.getElementById("userTotalDamage"),
  enemyTotalDamage: document.getElementById("enemyTotalDamage"),
  mvpName: document.getElementById("mvpName"),
  mvpDamage: document.getElementById("mvpDamage"),
  battleResult: document.getElementById("battleResult"),
  battleLogs: document.getElementById("battleLogs"),
  refreshDailyBtn: document.getElementById("refreshDailyBtn"),
  dailySigninBtn: document.getElementById("dailySigninBtn"),
  dailyClaimTaskBtn: document.getElementById("dailyClaimTaskBtn"),
  dailySignedIn: document.getElementById("dailySignedIn"),
  dailyBattleCount: document.getElementById("dailyBattleCount"),
  dailyTaskClaimed: document.getElementById("dailyTaskClaimed"),
  dailyMessage: document.getElementById("dailyMessage"),
  refreshLeaderboardBtn: document.getElementById("refreshLeaderboardBtn"),
  leaderboardList: document.getElementById("leaderboardList"),
  profileBackendTag: document.getElementById("profileBackendTag"),
  profileLevel: document.getElementById("profileLevel"),
  profileExp: document.getElementById("profileExp"),
  profileGold: document.getElementById("profileGold"),
  profileDiamond: document.getElementById("profileDiamond"),
  feedback: document.getElementById("feedback"),
};

function setActiveView(viewName) {
  state.activeView = ["home", "ranking", "battle"].includes(viewName) ? viewName : "home";
  const viewMap = {
    home: els.homeView,
    ranking: els.rankingView,
    battle: els.battleView,
  };
  Object.entries(viewMap).forEach(([name, element]) => {
    if (!element) {
      return;
    }
    element.classList.toggle("hidden", name !== state.activeView);
  });

  const navButtons = els.topNav?.querySelectorAll(".top-nav-btn") || [];
  navButtons.forEach((button) => {
    const isActive = button.dataset.view === state.activeView;
    button.classList.toggle("is-active", isActive);
    button.setAttribute("aria-selected", isActive ? "true" : "false");
  });
}

function renderProfile(profile, backendName) {
  const data = profile && typeof profile === "object" ? profile : null;
  if (els.profileLevel) {
    els.profileLevel.textContent = data ? String(data.level ?? 1) : "-";
  }
  if (els.profileExp) {
    els.profileExp.textContent = data ? String(data.exp ?? 0) : "-";
  }
  if (els.profileGold) {
    els.profileGold.textContent = data ? String(data.gold ?? 0) : "-";
  }
  if (els.profileDiamond) {
    els.profileDiamond.textContent = data ? String(data.diamond ?? 0) : "-";
  }
  if (els.profileBackendTag) {
    els.profileBackendTag.textContent = backendName || "-";
  }
}

function setDailyMessage(message) {
  if (!els.dailyMessage) {
    return;
  }
  els.dailyMessage.textContent = message;
}

function renderDailyState(daily) {
  const stateDaily = daily && typeof daily === "object" ? daily : {};
  const signedIn = Boolean(stateDaily.signed_in);
  const taskClaimed = Boolean(stateDaily.task_claimed);
  const battleCount = Number(stateDaily.battle_count || 0);
  const taskTarget = Number(stateDaily.task_target || 3);

  if (els.dailySignedIn) {
    els.dailySignedIn.textContent = signedIn ? "已签到" : "未签到";
  }
  if (els.dailyBattleCount) {
    els.dailyBattleCount.textContent = `${battleCount} / ${taskTarget}`;
  }
  if (els.dailyTaskClaimed) {
    els.dailyTaskClaimed.textContent = taskClaimed ? "已领取" : "未领取";
  }
  if (els.dailySigninBtn) {
    els.dailySigninBtn.disabled = signedIn;
  }
  if (els.dailyClaimTaskBtn) {
    els.dailyClaimTaskBtn.disabled = taskClaimed || battleCount < taskTarget;
  }
}

function renderLeaderboard(list) {
  if (!els.leaderboardList) {
    return;
  }
  els.leaderboardList.innerHTML = "";
  if (!Array.isArray(list) || list.length === 0) {
    const li = document.createElement("li");
    li.textContent = "暂无排行数据";
    els.leaderboardList.appendChild(li);
    return;
  }

  list.forEach((row, index) => {
    const rank = Number(row?.rank || index + 1);
    const account = row?.account || "-";
    const level = Number(row?.level || 1);
    const gold = Number(row?.gold || 0);

    const li = document.createElement("li");
    li.innerHTML = `
      <span class="rank-badge ${rank === 1 ? "rank-top1" : ""}">${rank}</span>
      <div class="leaderboard-main">
        <strong>${account}</strong>
        <small>Lv.${level}</small>
      </div>
      <div class="leaderboard-value">${gold}</div>
    `;
    els.leaderboardList.appendChild(li);
  });
}

async function refreshDailyState() {
  if (!state.account || !state.token) {
    renderDailyState(null);
    setDailyMessage("登录后可查看每日状态。");
    return;
  }
  try {
    const data = await apiRequest("/daily");
    state.lastDaily = data?.daily || null;
    state.profile = data?.profile || state.profile;
    renderDailyState(state.lastDaily);
    renderProfile(state.profile, data?.storage_backend);
    setDailyMessage("每日状态已更新");
  } catch (error) {
    setDailyMessage("每日状态拉取失败");
    setFeedback(error.payload || error.message);
  }
}

async function refreshLeaderboard() {
  try {
    const data = await apiRequest("/leaderboard", {
      headers: { "x-limit": "10" },
    });
    state.lastLeaderboard = Array.isArray(data?.ranking) ? data.ranking : [];
    renderLeaderboard(state.lastLeaderboard);
  } catch (error) {
    setFeedback(error.payload || error.message);
  }
}

async function doDailySignin() {
  try {
    const data = await apiRequest("/daily/signin", { method: "POST", body: {} });
    state.lastDaily = data?.daily || state.lastDaily;
    state.profile = data?.profile || state.profile;
    renderDailyState(state.lastDaily);
    renderProfile(state.profile, data?.storage_backend);
    setDailyMessage("签到成功，奖励已到账");
    setFeedback(data);
    await refreshLeaderboard();
  } catch (error) {
    const message = error.payload?.message === "already_signed_in" ? "今日已签到" : "签到失败";
    setDailyMessage(message);
    setFeedback(error.payload || error.message);
    await refreshDailyState();
  }
}

async function doDailyClaimTask() {
  try {
    const data = await apiRequest("/daily/claim-task", { method: "POST", body: {} });
    state.lastDaily = data?.daily || state.lastDaily;
    state.profile = data?.profile || state.profile;
    renderDailyState(state.lastDaily);
    renderProfile(state.profile, data?.storage_backend);
    setDailyMessage("任务奖励领取成功");
    setFeedback(data);
    await refreshLeaderboard();
  } catch (error) {
    const msgCode = error.payload?.message;
    const message = msgCode === "task_not_ready" ? "任务未完成：需当日完成 3 场战斗" : msgCode === "task_already_claimed" ? "任务奖励今日已领取" : "领取失败";
    setDailyMessage(message);
    setFeedback(error.payload || error.message);
    await refreshDailyState();
  }
}

function setFeedback(data) {
  if (!els.feedback) {
    return;
  }
  if (typeof data === "string") {
    els.feedback.textContent = data;
    return;
  }
  els.feedback.textContent = JSON.stringify(data, null, 2);
}

function setServerStatus(ok) {
  els.serverStatus.textContent = ok ? "服务在线" : "服务离线";
  els.serverStatus.classList.toggle("badge-ok", ok);
  els.serverStatus.classList.toggle("badge-warn", !ok);
}

function setResultBadge(result) {
  const text = result === "win" ? "胜利" : result === "lose" ? "失败" : result === "draw" ? "平局" : "未开始";
  els.resultBadge.textContent = text;
}

function getCharacterById(id) {
  return state.characters.find((character) => character.id === id) || null;
}

function getScenarioById(id) {
  return battleScenarios.find((scenario) => scenario.id === id) || battleScenarios[0];
}

function characterShortName(name) {
  if (!name) {
    return "--";
  }
  return name.length <= 2 ? name : name.slice(-2);
}

function heroImagePath(id) {
  return `/assets/heroes/${id}.png`;
}

async function apiRequest(path, options = {}) {
  const normalizedPath = path.startsWith("/") ? path : `/${path}`;
  const headers = {
    "Content-Type": "application/json",
    "x-account": state.account,
    "x-token": state.token,
    ...(options.headers || {}),
  };

  const response = await fetch(`${API_BASE}${normalizedPath}`, {
    method: options.method || "GET",
    headers,
    body: options.body ? JSON.stringify(options.body) : undefined,
  });

  const text = await response.text();
  let data = null;
  try {
    data = text ? JSON.parse(text) : null;
  } catch {
    data = { raw: text };
  }

  if (!response.ok) {
    const error = new Error(data?.error || `HTTP ${response.status}`);
    error.payload = data;
    throw error;
  }

  return data;
}

async function loadJsonFile(path) {
  const response = await fetch(path, { cache: "no-store" });
  if (!response.ok) {
    throw new Error(`load failed: ${path}`);
  }
  return response.json();
}

async function loadBattleData() {
  try {
    const [charactersData, itemsData] = await Promise.all([
      loadJsonFile("/characters.json"),
      loadJsonFile("/item_samples.json"),
    ]);

    if (Array.isArray(charactersData?.characters) && charactersData.characters.length > 0) {
      state.characters = charactersData.characters;
    }
    if (itemsData && typeof itemsData === "object") {
      state.itemSamples = itemsData;
    }
    setFeedback("已加载战斗配置数据：characters.json + item_samples.json");
  } catch {
    setFeedback("未读取到外部数据文件，已使用内置武将列表。可通过 load.sh 同步 json 到 /var/www/html");
  }
}

function renderTeamSlots(container, sideLabel, defaultIndices) {
  container.innerHTML = "";
  for (let index = 0; index < 3; index += 1) {
    const wrapper = document.createElement("div");
    wrapper.className = "slot-item";

    const title = document.createElement("b");
    title.textContent = `${sideLabel} ${index + 1} 号位`;

    const select = document.createElement("select");
    select.dataset.role = "general-select";

    state.characters.forEach((character) => {
      const option = document.createElement("option");
      option.value = String(character.id);
      option.textContent = `${character.name} [${character.position}] #${character.id}`;
      select.appendChild(option);
    });

    const defaultIndex = defaultIndices[index] ?? index;
    if (state.characters[defaultIndex]) {
      select.value = String(state.characters[defaultIndex].id);
    }

    wrapper.appendChild(title);
    wrapper.appendChild(select);
    container.appendChild(wrapper);
  }
}

function renderSelectFromArray(selectEl, dataList, formatFn) {
  selectEl.innerHTML = "";
  const emptyOption = document.createElement("option");
  emptyOption.value = "";
  emptyOption.textContent = "不选择";
  selectEl.appendChild(emptyOption);

  dataList.forEach((item) => {
    const option = document.createElement("option");
    option.value = String(item.id);
    option.textContent = formatFn(item);
    selectEl.appendChild(option);
  });
}

function renderItemDataPreview() {
  const items = Array.isArray(state.itemSamples?.items) ? state.itemSamples.items : [];
  const equips = Array.isArray(state.itemSamples?.equipments) ? state.itemSamples.equipments : [];
  const sets = Array.isArray(state.itemSamples?.setBonuses) ? state.itemSamples.setBonuses : [];

  renderSelectFromArray(
    els.userItemSelect,
    items,
    (item) => `${item.name} (${item.type || "ITEM"}) #${item.id}`,
  );
  renderSelectFromArray(
    els.enemyItemSelect,
    items,
    (item) => `${item.name} (${item.type || "ITEM"}) #${item.id}`,
  );
  renderSelectFromArray(
    els.userEquipSelect,
    equips,
    (equip) => `${equip.name} (${equip.type || "EQUIP"}) #${equip.id}`,
  );
  renderSelectFromArray(
    els.enemyEquipSelect,
    equips,
    (equip) => `${equip.name} (${equip.type || "EQUIP"}) #${equip.id}`,
  );

  els.dataPreview.innerHTML = "";
  const infoLines = [
    `道具数量: ${items.length}`,
    `装备数量: ${equips.length}`,
    `套装数量: ${sets.length}`,
  ];

  els.kpiGrid.innerHTML = "";
  const kpis = [
    { label: "道具", value: items.length },
    { label: "装备", value: equips.length },
    { label: "套装", value: sets.length },
  ];
  kpis.forEach((kpi) => {
    const item = document.createElement("div");
    item.className = "kpi";
    item.innerHTML = `<b>${kpi.value}</b><span>${kpi.label}</span>`;
    els.kpiGrid.appendChild(item);
  });

  infoLines.forEach((line) => {
    const li = document.createElement("li");
    li.textContent = line;
    els.dataPreview.appendChild(li);
  });
}

function collectTeamIds(container) {
  const ids = [];
  const selects = container.querySelectorAll("select[data-role='general-select']");
  selects.forEach((select) => {
    const value = Number(select.value);
    if (Number.isInteger(value) && value > 0) {
      ids.push(value);
    }
  });
  return ids;
}

function renderTeamPreview(container, teamIds) {
  container.innerHTML = "";
  teamIds.forEach((id) => {
    const character = getCharacterById(id);
    const card = document.createElement("div");
    card.className = "preview-card";

    const qualityClass = qualityClassMap[character?.quality] || "";
    const qualityText = character?.quality || "NORMAL";
    const position = character?.position || "UNKNOWN";
    const name = character?.name || `武将#${id}`;
    const avatarText = characterShortName(name);

    card.innerHTML = `
      <div class="avatar">
        <img src="${heroImagePath(id)}" alt="${name}" onerror="this.style.display='none'; this.nextElementSibling.style.display='grid';" />
        <span class="avatar-fallback">${avatarText}</span>
      </div>
      <div>
        <div class="preview-name">${name}</div>
        <small>${position} · ID ${id}</small>
      </div>
      <span class="quality ${qualityClass}">${qualityText}</span>
    `;

    container.appendChild(card);
  });
}

function setTeamByIds(container, teamIds) {
  const selects = container.querySelectorAll("select[data-role='general-select']");
  selects.forEach((select, index) => {
    if (teamIds[index]) {
      select.value = String(teamIds[index]);
    }
  });
}

function renderScenarioOptions() {
  els.scenarioSelect.innerHTML = "";
  battleScenarios.forEach((scenario) => {
    const option = document.createElement("option");
    option.value = scenario.id;
    option.textContent = scenario.name;
    els.scenarioSelect.appendChild(option);
  });
  els.scenarioSelect.value = battleScenarios[0].id;
  els.scenarioDesc.textContent = battleScenarios[0].desc;
}

function applyScenarioToEnemyTeam() {
  const scenario = getScenarioById(els.scenarioSelect.value);
  els.scenarioDesc.textContent = scenario.desc;
  setTeamByIds(els.enemyTeamSlots, scenario.enemyTeam);
  refreshBattleStagePreview();
}

function renderSetBonusPreview() {
  const equipIdList = [Number(els.userEquipSelect.value), Number(els.enemyEquipSelect.value)].filter((id) => Number.isInteger(id) && id > 0);
  const equipments = Array.isArray(state.itemSamples?.equipments) ? state.itemSamples.equipments : [];
  const setBonuses = Array.isArray(state.itemSamples?.setBonuses) ? state.itemSamples.setBonuses : [];

  const setCountMap = new Map();
  equipIdList.forEach((equipId) => {
    const equip = equipments.find((item) => item.id === equipId);
    if (!equip || !equip.setId) {
      return;
    }
    const prev = setCountMap.get(equip.setId) || 0;
    setCountMap.set(equip.setId, prev + 1);
  });

  els.setBonusPreview.innerHTML = "";
  if (setCountMap.size === 0) {
    const li = document.createElement("li");
    li.textContent = "未选择套装装备，暂无激活效果。";
    els.setBonusPreview.appendChild(li);
    return;
  }

  setCountMap.forEach((pieceCount, setId) => {
    const setInfo = setBonuses.find((item) => item.setId === setId);
    if (!setInfo) {
      return;
    }

    const activated = (setInfo.bonuses || []).filter((bonus) => pieceCount >= bonus.pieceCount);
    const li = document.createElement("li");
    if (activated.length === 0) {
      li.textContent = `${setInfo.name} (${pieceCount}件)：未达到激活条件`;
    } else {
      li.textContent = `${setInfo.name} (${pieceCount}件)：${activated.map((bonus) => bonus.desc).join("；")}`;
    }
    els.setBonusPreview.appendChild(li);
  });
}

function closeBattleResultModal() {
  if (!els.battleResultModal) {
    return;
  }
  els.battleResultModal.classList.add("hidden");
  els.battleResultModal.setAttribute("aria-hidden", "true");
  document.body.classList.remove("modal-open");
}

function openBattleResultModal() {
  if (!els.battleResultModal) {
    return;
  }
  els.battleResultModal.classList.remove("hidden");
  els.battleResultModal.setAttribute("aria-hidden", "false");
  document.body.classList.add("modal-open");
}

function setDamageSideTab(side) {
  state.damageSideTab = side === "enemy" ? "enemy" : "user";
  const tabButtons = els.damageSideTabs?.querySelectorAll(".damage-tab") || [];
  tabButtons.forEach((button) => {
    const isActive = button.dataset.side === state.damageSideTab;
    button.classList.toggle("is-active", isActive);
    button.setAttribute("aria-selected", isActive ? "true" : "false");
  });
}

function normalizeDamageSide(side) {
  const value = String(side || "").toLowerCase();
  if (value === "enemy" || value === "foe") {
    return "enemy";
  }
  return "user";
}

function renderBattleLogs(logs) {
  els.battleLogs.innerHTML = "";
  if (!Array.isArray(logs) || logs.length === 0) {
    const li = document.createElement("li");
    li.textContent = "暂无日志";
    els.battleLogs.appendChild(li);
    return;
  }

  logs.forEach((line) => {
    const li = document.createElement("li");
    li.textContent = String(line);
    if (/回合|result|胜|负|战斗/.test(li.textContent)) {
      li.classList.add("log-highlight");
    }
    if (/失败|error|invalid|未/.test(li.textContent)) {
      li.classList.add("log-warning");
    }
    els.battleLogs.appendChild(li);
  });
}

function renderDamageBoard(board) {
  if (!els.damageBoardList) {
    return;
  }
  els.damageBoardList.innerHTML = "";
  if (!Array.isArray(board) || board.length === 0) {
    const li = document.createElement("li");
    li.textContent = "未获取到伤害统计";
    els.damageBoardList.appendChild(li);
    return;
  }

  let sideBoard = board
    .filter((row) => normalizeDamageSide(row?.side) === state.damageSideTab)
    .sort((left, right) => Number(right?.damage || 0) - Number(left?.damage || 0));

  if (sideBoard.length === 0) {
    sideBoard = [...board].sort((left, right) => Number(right?.damage || 0) - Number(left?.damage || 0));
  }

  const maxDamage = Math.max(...sideBoard.map((row) => Number(row?.damage || 0)), 1);
  const mvpName = String(state.lastDamageSummary?.mvp_name || "");
  const mvpDamage = Number(state.lastDamageSummary?.mvp_damage || 0);

  sideBoard.forEach((row) => {
    const damage = Number(row?.damage || 0);
    const progress = Math.max(4, Math.round((damage / maxDamage) * 100));
    const li = document.createElement("li");
    const normalizedSide = normalizeDamageSide(row?.side);
    const side = normalizedSide === "user" ? "我方" : "敌方";
    li.className = normalizedSide === "user" ? "damage-user" : "damage-enemy";
    const isMvp = row.name === mvpName && damage === mvpDamage;
    if (isMvp) {
      li.classList.add("damage-mvp");
    }
    li.innerHTML = `
      <div class="damage-main">
        <div class="damage-top">
          <div>
            <div class="damage-name">${row.name || "未知角色"}${isMvp ? '<span class="damage-tag">MVP</span>' : ""}</div>
            <div class="damage-side">${side} · ${row.alive ? "存活" : "阵亡"}</div>
          </div>
          <div class="damage-value">${damage}</div>
        </div>
        <div class="damage-progress-track">
          <div class="damage-progress-fill" style="width: ${progress}%"></div>
        </div>
      </div>
    `;
    els.damageBoardList.appendChild(li);
  });
}

function handleDamageSideTabClick(event) {
  const tabButton = event.target.closest(".damage-tab");
  if (!tabButton) {
    return;
  }
  const selectedSide = tabButton.dataset.side;
  setDamageSideTab(selectedSide);
  renderDamageBoard(state.lastDamageBoard);
}

function handleGlobalKeydown(event) {
  if (event.key !== "Escape") {
    return;
  }
  if (!els.battleResultModal || els.battleResultModal.classList.contains("hidden")) {
    return;
  }
  closeBattleResultModal();
}

function toggleBattleProcess() {
  state.processExpanded = !state.processExpanded;
  els.battleProcessSection.classList.toggle("hidden", !state.processExpanded);
  els.toggleBattleProcessBtn.textContent = state.processExpanded ? "收起战斗过程" : "查看战斗过程";
  if (state.processExpanded) {
    renderBattleLogs(state.lastBattleLogs);
  }
}

function refreshBattleStagePreview() {
  const userTeam = collectTeamIds(els.userTeamSlots);
  const enemyTeam = collectTeamIds(els.enemyTeamSlots);
  renderTeamPreview(els.userTeamPreview, userTeam);
  renderTeamPreview(els.enemyTeamPreview, enemyTeam);
}

function updateAuthUi() {
  const authed = Boolean(state.account && state.token);
  els.loginPanel.classList.toggle("hidden", authed);
  els.gamePanel.classList.toggle("hidden", !authed);
  els.accountBadge.textContent = authed ? `账号: ${state.account}` : "未登录";
  els.accountInput.value = state.account;
  els.tokenInput.value = state.token;
  if (!authed) {
    setActiveView("home");
  }
}

function updateResultUi(session, logs, damageBoard, damageSummary) {
  els.resultPanel.classList.remove("hidden");
  state.lastBattleLogs = Array.isArray(logs) ? logs : [];
  state.lastDamageBoard = Array.isArray(damageBoard) ? damageBoard : [];
  state.lastDamageSummary = damageSummary && typeof damageSummary === "object" ? damageSummary : null;
  state.processExpanded = false;
  els.battleProcessSection.classList.add("hidden");
  els.toggleBattleProcessBtn.textContent = "查看战斗过程";
  setDamageSideTab("user");

  els.round.textContent = String(session?.round ?? 0);
  els.userTotalDamage.textContent = String(damageSummary?.user_total ?? 0);
  els.enemyTotalDamage.textContent = String(damageSummary?.enemy_total ?? 0);
  els.mvpName.textContent = String(damageSummary?.mvp_name || "-");
  els.mvpDamage.textContent = String(damageSummary?.mvp_damage ?? 0);
  els.battleResult.textContent = session?.result || "-";
  setResultBadge(session?.result || "-");

  const resultMap = {
    win: "我方大获全胜，战术执行完美。",
    lose: "本场失利，建议调整阵容克制关系。",
    draw: "势均力敌，可提高输出位强度。",
  };
  const headline = resultMap[session?.result] || "战斗结束，已生成完整战报。";
  els.resultHeadline.textContent = headline;
  els.modalResultHeadline.textContent = headline;
  els.modalResultTitle.textContent = session?.result === "win" ? "战斗胜利" : session?.result === "lose" ? "战斗失败" : "战斗结束";
  renderDamageBoard(state.lastDamageBoard);
  openBattleResultModal();
}

async function probeServer() {
  try {
    await apiRequest("/ping");
    setServerStatus(true);
  } catch {
    setServerStatus(false);
  }
}

async function doLogin(event) {
  event.preventDefault();
  const account = els.accountInput.value.trim();
  const token = els.tokenInput.value.trim();

  if (!account || !token) {
    setFeedback("账号和 Token 不能为空");
    return;
  }

  state.account = account;
  state.token = token;

  try {
    const data = await apiRequest("/login", { method: "POST", body: {} });
    localStorage.setItem("wg_account", state.account);
    localStorage.setItem("wg_token", state.token);
    state.profile = data?.profile || null;
    updateAuthUi();
    renderProfile(state.profile, data?.storage_backend);
    setActiveView("home");
    await refreshDailyState();
    await refreshLeaderboard();
    setFeedback(data);
  } catch (error) {
    setFeedback(error.payload || error.message);
  }
}

async function runAutoBattle() {
  const userTeam = collectTeamIds(els.userTeamSlots);
  const enemyTeam = collectTeamIds(els.enemyTeamSlots);

  if (userTeam.length === 0 || enemyTeam.length === 0) {
    setFeedback("请至少配置我方和敌方各 1 名武将");
    return;
  }

  const payload = {
    action: "auto",
    user_team: userTeam,
    enemy_team: enemyTeam,
    loadout: {
      user_item_id: Number(els.userItemSelect.value) || 0,
      enemy_item_id: Number(els.enemyItemSelect.value) || 0,
      user_equip_id: Number(els.userEquipSelect.value) || 0,
      enemy_equip_id: Number(els.enemyEquipSelect.value) || 0,
    },
  };

  try {
    els.runAutoBtn.disabled = true;
    els.runAutoBtn.textContent = "战斗计算中...";
    const data = await apiRequest("/battle", {
      method: "POST",
      body: payload,
    });
    state.profile = data?.profile_after || state.profile;
    updateResultUi(data.session, data.battle_logs, data.damage_board, data.damage_summary);
    renderProfile(state.profile, data?.storage_backend);
    await refreshDailyState();
    await refreshLeaderboard();
    setFeedback(data);
  } catch (error) {
    setFeedback(error.payload || error.message);
  } finally {
    els.runAutoBtn.disabled = false;
    els.runAutoBtn.textContent = "开始自动战斗";
  }
}

function logout() {
  state.account = "";
  state.token = "";
  state.lastDamageBoard = [];
  state.lastDamageSummary = null;
  state.lastDaily = null;
  state.lastLeaderboard = [];
  state.profile = null;
  localStorage.removeItem("wg_account");
  localStorage.removeItem("wg_token");
  updateAuthUi();
  els.resultPanel.classList.add("hidden");
  closeBattleResultModal();
  setResultBadge("-");
  els.resultHeadline.textContent = "等待战斗开始...";
  renderDailyState(null);
  renderLeaderboard([]);
  renderProfile(null, "-");
  setActiveView("home");
  setDailyMessage("登录后可查看每日状态。");
  setFeedback("已退出登录");
}

function renderConfigUi() {
  renderTeamSlots(els.userTeamSlots, "我方", [0, 1, 2]);
  renderTeamSlots(els.enemyTeamSlots, "敌方", [3, 4, 5]);
  renderScenarioOptions();
  renderItemDataPreview();
  applyScenarioToEnemyTeam();
  renderSetBonusPreview();
  refreshBattleStagePreview();
  renderDailyState(null);
  renderLeaderboard([]);
  renderProfile(null, "-");
}

function handleTopNavClick(event) {
  const button = event.target.closest(".top-nav-btn");
  if (!button) {
    return;
  }
  setActiveView(button.dataset.view);
}

function bindEvents() {
  els.loginForm?.addEventListener("submit", doLogin);
  els.logoutBtn?.addEventListener("click", logout);
  els.runAutoBtn?.addEventListener("click", runAutoBattle);
  els.closeResultModalBtn?.addEventListener("click", closeBattleResultModal);
  els.closeResultModalBtn?.addEventListener("pointerup", closeBattleResultModal);
  els.toggleBattleProcessBtn?.addEventListener("click", toggleBattleProcess);
  els.battleResultModalCard?.addEventListener("click", (event) => {
    event.stopPropagation();
  });
  els.battleResultModal?.addEventListener("click", (event) => {
    if (event.target === els.battleResultModal) {
      closeBattleResultModal();
    }
  });
  els.damageSideTabs?.addEventListener("click", handleDamageSideTabClick);
  document.addEventListener("keydown", handleGlobalKeydown);
  els.userTeamSlots?.addEventListener("change", refreshBattleStagePreview);
  els.enemyTeamSlots?.addEventListener("change", refreshBattleStagePreview);
  els.scenarioSelect?.addEventListener("change", applyScenarioToEnemyTeam);
  els.userEquipSelect?.addEventListener("change", renderSetBonusPreview);
  els.enemyEquipSelect?.addEventListener("change", renderSetBonusPreview);
  els.refreshDailyBtn?.addEventListener("click", refreshDailyState);
  els.dailySigninBtn?.addEventListener("click", doDailySignin);
  els.dailyClaimTaskBtn?.addEventListener("click", doDailyClaimTask);
  els.refreshLeaderboardBtn?.addEventListener("click", refreshLeaderboard);
  els.topNav?.addEventListener("click", handleTopNavClick);
}

async function bootstrap() {
  bindEvents();
  closeBattleResultModal();
  updateAuthUi();
  await loadBattleData();
  renderConfigUi();
  setActiveView("home");
  await refreshLeaderboard();
  await refreshDailyState();
  await probeServer();
}

bootstrap();
