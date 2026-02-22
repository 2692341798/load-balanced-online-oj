# Training List Module Checklist

## 1. Database
- [ ] `training_lists` table created successfully.
- [ ] `training_list_items` table created successfully.
- [ ] Constraints (Foreign Keys, Unique Keys) are correct.

## 2. Backend
- [ ] `POST /api/training/create` returns success and new ID.
- [ ] `GET /api/training/list` returns list of public training lists.
- [ ] `GET /api/training/{id}` returns correct list details and problems.
- [ ] `POST /api/training/add_problem` adds problem correctly.
- [ ] `POST /api/training/remove_problem` removes problem correctly.
- [ ] `POST /api/training/reorder` updates order correctly.
- [ ] `POST /api/training/edit` updates title/desc correctly.
- [ ] `POST /api/training/delete` deletes list and cascade deletes items.
- [ ] Only author can edit/delete their own list.
- [ ] Public lists are visible to everyone; Private lists only to author.

## 3. Frontend
- [ ] `/training` page loads successfully.
- [ ] Grid layout displays training cards correctly (Title, Author, Difficulty, Count).
- [ ] Clicking a card navigates to `/training/{id}`.
- [ ] Detail page shows list info and problems.
- [ ] Create button opens modal/page to create new list.
- [ ] Edit mode allows reordering problems (Drag & Drop).
- [ ] Search/Filter by difficulty works.
- [ ] Progress bar reflects user's solved status.

## 4. Performance & UX
- [ ] Page load time < 100ms for list view (cached or efficient query).
- [ ] Mobile view is responsive (cards stack vertically).
- [ ] Empty state shown when no lists exist.
- [ ] Loading states on buttons during API calls.
