
-- Create a simple key-value table
CREATE TABLE IF NOT EXISTS kvstore (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

-- Insert the key-value pair
INSERT INTO kvstore (key, value)
VALUES ('hello', 'world')
ON CONFLICT (key) DO UPDATE
SET value = EXCLUDED.value;

-- Verify
SELECT * FROM kvstore;
