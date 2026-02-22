# Training List Module Tasks

## Phase 1: Database Setup
- [ ] Create `training_lists` table in MySQL.
- [ ] Create `training_list_items` table in MySQL.
- [ ] (Optional) Create `training_list_likes` table.
- [ ] Verify database connectivity and schema correctness.

## Phase 2: Backend Model Implementation (oj_server/oj_model.hpp)
- [ ] Define `TrainingList` struct/class.
- [ ] Implement `CreateList` method.
- [ ] Implement `GetList` method (retrieve by ID).
- [ ] Implement `UpdateList` method.
- [ ] Implement `DeleteList` method.
- [ ] Implement `AddProblemToList` method.
- [ ] Implement `RemoveProblemFromList` method.
- [ ] Implement `ReorderProblems` method.
- [ ] Implement `GetListProblems` method (retrieve ordered problems for a list).

## Phase 3: Backend API Implementation (oj_server/oj_server.cc)
- [ ] Create route handler for `POST /api/training/create`.
- [ ] Create route handler for `POST /api/training/edit`.
- [ ] Create route handler for `POST /api/training/delete`.
- [ ] Create route handler for `POST /api/training/add_problem`.
- [ ] Create route handler for `POST /api/training/remove_problem`.
- [ ] Create route handler for `POST /api/training/reorder`.
- [ ] Create route handler for `GET /api/training/list`.
- [ ] Create route handler for `GET /api/training/{id}`.

## Phase 4: Frontend Implementation (Templates & CSS)
- [ ] Create `training_list.html` template (list view).
- [ ] Create `training_detail.html` template (detail view).
- [ ] Update `index.css` with new styles for training lists (cards, progress bar, etc.).
- [ ] Add navigation link to `/training` in `header.html` or main layout.
- [ ] Implement JS for creating/editing lists (AJAX calls).
- [ ] Implement JS for adding/removing/reordering problems (Drag & Drop with SortableJS).

## Phase 5: Testing & Refinement
- [ ] Verify CRUD operations via Postman/curl.
- [ ] Test permissions (ensure only author can edit/delete).
- [ ] Test public/private visibility.
- [ ] Verify responsive design on mobile/desktop.
- [ ] Performance check (loading time < 100ms for list view).
- [ ] Fix any bugs found during testing.

## Phase 6: Documentation
- [ ] Update `api_reference.md` with new endpoints.
- [ ] Update `database.md` with new schema.
