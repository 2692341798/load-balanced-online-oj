# Training List Module Specification

## 1. Overview
The Training List module allows users to create, manage, and share collections of problems ("Training Lists"). It supports creating personalized lists from the existing problem bank, sharing them publicly, and tracking progress.

## 2. Database Schema

### `training_lists` Table
Stores metadata for training lists.

```sql
CREATE TABLE IF NOT EXISTS training_lists (
    id INT PRIMARY KEY AUTO_INCREMENT,
    title VARCHAR(255) NOT NULL,
    description TEXT,
    difficulty VARCHAR(50) DEFAULT 'Unrated', -- e.g., 'Easy', 'Medium', 'Hard'
    tags JSON, -- Stores tags as JSON array, e.g., ["DP", "Graph"]
    author_id INT NOT NULL,
    visibility ENUM('public', 'private') DEFAULT 'public',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    likes INT DEFAULT 0,
    collections INT DEFAULT 0,
    FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
);
```

### `training_list_items` Table
Maps questions to training lists with ordering.

```sql
CREATE TABLE IF NOT EXISTS training_list_items (
    id INT PRIMARY KEY AUTO_INCREMENT,
    training_list_id INT NOT NULL,
    question_id INT NOT NULL,
    order_index INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (training_list_id) REFERENCES training_lists(id) ON DELETE CASCADE,
    FOREIGN KEY (question_id) REFERENCES oj_questions(id) ON DELETE CASCADE,
    UNIQUE KEY unique_item (training_list_id, question_id)
);
```

### `training_list_likes` Table (Optional for Phase 1)
Tracks user likes on training lists.

```sql
CREATE TABLE IF NOT EXISTS training_list_likes (
    id INT PRIMARY KEY AUTO_INCREMENT,
    training_list_id INT NOT NULL,
    user_id INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY unique_like (training_list_id, user_id)
);
```

## 3. Backend API (oj_server)

### endpoints

#### List Management
- `POST /api/training/create`
  - Input: `{ "title": "...", "description": "...", "difficulty": "...", "tags": [...], "visibility": "public" }`
  - Output: `{ "id": 123, "status": "success" }`
- `POST /api/training/edit`
  - Input: `{ "id": 123, "title": "...", ... }`
  - Output: `{ "status": "success" }`
- `POST /api/training/delete`
  - Input: `{ "id": 123 }`
  - Output: `{ "status": "success" }`

#### Problem Management within List
- `POST /api/training/add_problem`
  - Input: `{ "training_list_id": 123, "question_id": 456 }`
  - Output: `{ "status": "success" }`
- `POST /api/training/remove_problem`
  - Input: `{ "training_list_id": 123, "question_id": 456 }`
  - Output: `{ "status": "success" }`
- `POST /api/training/reorder`
  - Input: `{ "training_list_id": 123, "problem_ids": [456, 789, ...] }`
  - Output: `{ "status": "success" }`

#### Data Retrieval
- `GET /api/training/list`
  - Query Params: `page`, `limit`, `visibility`, `author_id`
  - Output: JSON array of training lists with metadata.
- `GET /api/training/{id}`
  - Output: detailed JSON of training list + array of problems (with user's solve status).

## 4. Frontend Implementation

### Pages
1.  **Training List Index** (`/training`)
    - Displays grid of public training lists.
    - Filters: Difficulty, Tags, Author.
    - User's own lists section (if logged in).
2.  **Training List Detail** (`/training/{id}`)
    - Header: Title, Desc, Author, Progress Bar.
    - Body: List of problems (ID, Title, Difficulty, Status).
    - Sidebar: Actions (Edit, Delete - if author), Social (Like, Collect).
3.  **Training List Editor** (Integrated into Detail or separate `/training/{id}/edit`)
    - Drag-and-drop interface for reordering.
    - Search bar to find and add problems from the main question bank.

### Components
- **Card Component**: For displaying training list summary.
- **Problem List Item**: Row in the detail view.
- **Progress Bar**: Visual indicator of completion.
- **SortableJS Integration**: For drag-and-drop reordering.

## 5. Security & Permissions
- **Creation**: Login required.
- **Editing/Deletion**: Only author (checked via session `user_id` vs `training_list.author_id`).
- **Visibility**: Private lists only accessible by author. Public lists accessible by all.

## 6. Performance
- **Caching**: Cache public training list metadata (LRU or simple time-based).
- **Pagination**: Limit 20 lists per page.
- **Problem Status**: Fetch user's solved problem IDs separately and map in frontend or efficiently join in backend.
