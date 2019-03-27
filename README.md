Fully Compliant AIML 1.0 Interpreter With Cherry Picked AIML 2.0 Features.

The end-goal is to be fully compliant with AIML 2.x

This is a WIP with many features still left to implement.

Working off of (http://callmom.pandorabots.com/static/reference/) 

### Pattern Matching Priority:
###### `@` Priority Word
###### `#` 0+ Priority Wildcard
###### `_` 1+ Priority Wildcard
###### `ABC123` Word
###### `^` 0+ Wildcard
###### `*` 1+ Wildcard

### Completed:
###### `<aiml>`
###### `<category>`
###### `<li>` *as a child of `<random>`
###### `<pattern>`
###### `<random>`
###### `<star />`
###### `<template>`
###### `<think>`
###### `<topic>` *Not yet if defined inside `<category>`
###### `<set name="PREDICATE_NAME">`
###### `<get name="PREDICATE_NAME" />`

### ToDo:
###### `<bot name="PROPERTY_NAME" />`
###### `<br />`
###### `<condition name="PREDICATE_NAME">`
###### `<date />`
###### `<denormalize>`
###### `<eval>`
###### `<explode>`
###### `<first>`
###### `<formal>`
###### `<gender>`
###### `<id />`
###### `<img src="http://image.source.format" />`
###### `<input /'>`
###### `<interval>`
###### `<learn>` *to be highly sophisticated
###### `<learnf>` *to be highly sophisticated
###### `<li>` *as a child of `<condition>`
###### `<loop />`
###### `<lowercase>`
###### `<map>`
###### `<normalize>`
###### `<person>`
###### `<person2>`
###### `<program />`
###### `<request />`
###### `<response />`
###### `<rest>`
###### `<sentence>`
###### `<size />`
###### `<sr />`
###### `<srai>`
###### `<sraix />`
###### `<that>`
###### `<thatstar>`
###### `<topicstar />`
###### `<uppercase>`

### Skipped:
###### `<oob>` *Has no current forseen future usage
