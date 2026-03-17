SELECT 'CREATE USER webgame WITH PASSWORD ''webgame''' 
WHERE NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'webgame')\gexec

SELECT 'CREATE DATABASE webgame OWNER webgame'
WHERE NOT EXISTS (SELECT 1 FROM pg_database WHERE datname = 'webgame')\gexec

\connect webgame

GRANT CONNECT ON DATABASE webgame TO webgame;
GRANT USAGE, CREATE ON SCHEMA public TO webgame;

CREATE TABLE IF NOT EXISTS user_profiles (
    account VARCHAR(64) PRIMARY KEY,
    level INTEGER NOT NULL DEFAULT 1,
    exp BIGINT NOT NULL DEFAULT 0,
    gold BIGINT NOT NULL DEFAULT 1000,
    diamond BIGINT NOT NULL DEFAULT 0,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS battle_records (
    id BIGSERIAL PRIMARY KEY,
    account VARCHAR(64) NOT NULL,
    battle_id VARCHAR(128) NOT NULL,
    result VARCHAR(16) NOT NULL,
    rounds INTEGER NOT NULL DEFAULT 0,
    user_total_damage BIGINT NOT NULL DEFAULT 0,
    enemy_total_damage BIGINT NOT NULL DEFAULT 0,
    reward_exp BIGINT NOT NULL DEFAULT 0,
    reward_gold BIGINT NOT NULL DEFAULT 0,
    reward_diamond BIGINT NOT NULL DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS daily_states (
    account VARCHAR(64) PRIMARY KEY,
    day_key BIGINT NOT NULL DEFAULT 0,
    signed_in BOOLEAN NOT NULL DEFAULT FALSE,
    battle_count INTEGER NOT NULL DEFAULT 0,
    task_claimed BOOLEAN NOT NULL DEFAULT FALSE,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_battle_records_account_created_at
    ON battle_records(account, created_at DESC);

INSERT INTO user_profiles(account, level, exp, gold, diamond)
VALUES
    ('user1', 3, 30, 1500, 12),
    ('user2', 2, 40, 1200, 8)
ON CONFLICT(account) DO NOTHING;

INSERT INTO battle_records(account, battle_id, result, rounds, user_total_damage, enemy_total_damage, reward_exp, reward_gold, reward_diamond)
SELECT * FROM (
    SELECT 'user1'::VARCHAR(64), 'seed-user1-1'::VARCHAR(128), 'win'::VARCHAR(16), 12, 32000::BIGINT, 18000::BIGINT, 180::BIGINT, 130::BIGINT, 2::BIGINT
    WHERE NOT EXISTS (SELECT 1 FROM battle_records WHERE battle_id = 'seed-user1-1')
    UNION ALL
    SELECT 'user1'::VARCHAR(64), 'seed-user1-2'::VARCHAR(128), 'lose'::VARCHAR(16), 9, 16000::BIGINT, 24000::BIGINT, 120::BIGINT, 95::BIGINT, 0::BIGINT
    WHERE NOT EXISTS (SELECT 1 FROM battle_records WHERE battle_id = 'seed-user1-2')
) AS seed_rows;

INSERT INTO daily_states(account, day_key, signed_in, battle_count, task_claimed)
SELECT account,
       FLOOR(EXTRACT(EPOCH FROM NOW()) / 86400)::BIGINT,
       FALSE,
       0,
       FALSE
FROM user_profiles
ON CONFLICT(account) DO NOTHING;
