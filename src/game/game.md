```plaintext
src/
├── core/             # 核心引擎/框架（网络、日志、配置、线程、定时器）
├── entity/           # 实体：Player、Hero、Monster、Item、Skill 等
├── battle/           # 战斗核心（战斗流程、伤害计算、Buff、回合逻辑）
├── scene/            # 场景/地图管理（大世界、城镇、副本场景基类）
├── game/             # 游戏业务顶层
│   ├── play/         # 【所有玩法统一放这里】副本、竞技场、爬塔、活动…
│   │   ├── dungeon/  # 副本（普通副本、精英副本、剧情副本）
│   │   ├── arena/    # 竞技场（实时/排行榜/匹配/奖励）
│   │   ├── tower/    # 爬塔、试炼之塔
│   │   ├── activity/ # 限时活动、节日活动、每日任务
│   │   ├── guild/    # 公会/帮派玩法（公会副本、公会战）
│   │   └── worldboss/# 世界BOSS
│   ├── task/         # 任务系统（主线、支线、日常）
│   ├── shop/         # 商城
│   └── mail/         # 邮件
├── config/           # 配置读取（JSON/Lua/CSV 解析）
├── net/              # 网络协议、消息处理
├── utils/            # 工具类（字符串、数学、随机）
└── main/             # 入口、启动逻辑
```
