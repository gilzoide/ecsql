-- Yoga + Text integration
CREATE TRIGGER YogaNode_OnInsert
AFTER INSERT ON YogaNode
BEGIN
  UPDATE YogaNode
  SET is_text_dirty = exists(SELECT entity_id FROM Text WHERE entity_id = new.entity_id)
  WHERE entity_id = new.entity_id;
END;

CREATE TRIGGER YogaNode_OnInsertText
AFTER INSERT ON Text
BEGIN
  UPDATE YogaNode
  SET is_text_dirty = TRUE
  WHERE entity_id = new.entity_id;
END;

CREATE TRIGGER YogaNode_OnUpdateText
AFTER UPDATE ON Text
BEGIN
  UPDATE YogaNode
  SET is_text_dirty = TRUE
  WHERE entity_id = new.entity_id;
END;

CREATE TRIGGER YogaNode_OnDeleteText
AFTER DELETE ON Text
BEGIN
  UPDATE YogaNode
  SET is_text_dirty = FALSE
  WHERE entity_id = old.entity_id;
END;
