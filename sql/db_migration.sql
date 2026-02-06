-- Migration script for Contest Module
-- Version: 1.0
-- Date: 2026-01-31

-- Create contests table
CREATE TABLE IF NOT EXISTS `contests` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `contest_id` varchar(50) NOT NULL COMMENT 'Platform ID',
    `name` varchar(255) NOT NULL,
    `start_time` datetime NOT NULL,
    `end_time` datetime NOT NULL,
    `link` varchar(255) NOT NULL,
    `source` varchar(50) DEFAULT 'Codeforces',
    `status` varchar(20) DEFAULT 'upcoming',
    `last_crawl_time` datetime DEFAULT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `idx_source_id` (`source`, `contest_id`),
    INDEX `idx_status_time` (`status`, `start_time` DESC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
