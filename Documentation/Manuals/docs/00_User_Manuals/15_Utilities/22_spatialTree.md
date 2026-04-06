---
title: Spatial Tree
---

@b3d::TSpatialTree is a template for creating spatial partitioning data structures - quad-trees for 2D space and oct-trees for 3D space. These structures efficiently organize spatial data to enable fast queries for elements within specific regions or overlapping certain bounds.

# Creating a spatial tree

## Defining options

Before creating a tree, define an options class that specifies tree behavior:

~~~~~~~~~~~~~{.cpp}
struct MyTreeOptions
{
	enum
	{
		LoosePadding = 4,
		MinimumElementsPerNode = 8,
		MaximumElementsPerNode = 16,
		MaximumDepth = 8
	};

	static simd::AABox GetBounds(const GameObject& element, void* context)
	{
		return element.GetWorldBounds();
	}

	static void SetElementId(const GameObject& element, const SpatialTreeElementId& elementId, void* context)
	{
		element.SetSpatialTreeId(elementId);
	}
};
~~~~~~~~~~~~~

The options class must provide:

- **LoosePadding** - Denominator for calculating node padding. Padding is (1.0f / LoosePadding). Higher values reduce padding but may cause elements to get stuck on parent nodes when straddling boundaries.

- **MinimumElementsPerNode** - When element count drops below this on removal, the node collapses and moves elements back to the parent.

- **MaximumElementsPerNode** - When element count exceeds this, the node subdivides into children (unless at maximum depth).

- **MaximumDepth** - Maximum tree depth. Nodes at this depth will not subdivide regardless of element count.

- **GetBounds(element, context)** - Returns SIMD bounds for an element.

- **SetElementId(element, elementId, context)** - Called when an element's ID is assigned or changed.

## Creating an octree (3D)

Use @b3d::TOctTree for 3D spatial partitioning:

~~~~~~~~~~~~~{.cpp}
// Create octree centered at origin with 1000 unit extent
Vector3 center(0.0f, 0.0f, 0.0f);
float extent = 1000.0f;

TOctTree<GameObject, MyTreeOptions> tree(center, extent);
~~~~~~~~~~~~~

## Creating a quadtree (2D)

Use @b3d::TQuadTree for 2D spatial partitioning:

~~~~~~~~~~~~~{.cpp}
// Create quadtree centered at origin with 500 unit extent
Vector2 center(0.0f, 0.0f);
float extent = 500.0f;

TQuadTree<Entity2D, MyTree2DOptions> tree(center, extent);
~~~~~~~~~~~~~

# Adding and removing elements

## Adding elements

Use @b3d::TSpatialTree::AddElement to insert elements:

~~~~~~~~~~~~~{.cpp}
TOctTree<GameObject, MyTreeOptions> tree(Vector3::kZero, 1000.0f);

GameObject player;
player.SetPosition(Vector3(10.0f, 5.0f, 0.0f));

tree.AddElement(player);
~~~~~~~~~~~~~

The tree automatically determines the appropriate node for the element based on its bounds.

## Removing elements

Use @b3d::TSpatialTree::RemoveElement with the element's @b3d::SpatialTreeElementId:

~~~~~~~~~~~~~{.cpp}
SpatialTreeElementId elementId = player.GetSpatialTreeId();

tree.RemoveElement(elementId);
~~~~~~~~~~~~~

The @b3d::SpatialTreeElementId is provided to your element via the SetElementId() method in your options class.

# Querying the tree

## Iterating all elements

Use @b3d::TSpatialTree::TreeIterator to iterate over every element:

~~~~~~~~~~~~~{.cpp}
TOctTree<GameObject, MyTreeOptions>::TreeIterator iterator(tree);

while(iterator.MoveNext())
{
	const GameObject& element = iterator.GetElement();
	B3D_LOG(Info, LogGeneric, "Element at: {0}", element.GetPosition());
}
~~~~~~~~~~~~~

## Querying by area intersection

Use @b3d::TSpatialTree::AreaIntersectIterator to find elements intersecting specific bounds:

~~~~~~~~~~~~~{.cpp}
// Find all objects within a 50-unit box around the player
AABox queryBounds(playerPosition, Vector3(50.0f, 50.0f, 50.0f));

TOctTree<GameObject, MyTreeOptions>::AreaIntersectIterator iterator(tree, queryBounds);

while(iterator.MoveNext())
{
	const GameObject& element = iterator.GetElement();
	ProcessNearbyObject(element);
}
~~~~~~~~~~~~~

# Advanced traversal

## Manual node iteration

Use @b3d::TSpatialTree::NodeIterator for custom tree traversal:

~~~~~~~~~~~~~{.cpp}
TOctTree<GameObject, MyTreeOptions>::NodeIterator nodeIterator(tree);

while(nodeIterator.MoveNext())
{
	const auto& nodeContext = nodeIterator.GetCurrent();
	const auto* node = nodeContext.Node;

	// Process elements in this node
	TOctTree<GameObject, MyTreeOptions>::ElementIterator elementIterator(node);
	while(elementIterator.MoveNext())
	{
		const GameObject& element = elementIterator.GetElement();
		ProcessElement(element);
	}

	// Optionally traverse child nodes
	auto childRange = node->GetChildren();
	for(u32 i = 0; i < 8; i++) // 8 children for octree
	{
		if(childRange.Contains(i) && node->HasChild(i))
		{
			nodeIterator.PushChild(i);
		}
	}
}
~~~~~~~~~~~~~

## Conditional traversal

~~~~~~~~~~~~~{.cpp}
AABox frustumBounds = CalculateFrustumBounds(camera);

TOctTree<GameObject, MyTreeOptions>::NodeIterator nodeIterator(tree);

while(nodeIterator.MoveNext())
{
	const auto& nodeContext = nodeIterator.GetCurrent();
	const AABox& nodeBounds = nodeContext.Bounds.GetBounds();

	// Skip nodes completely outside the frustum
	if(!frustumBounds.Overlaps(nodeBounds))
		continue;

	// Process elements in visible nodes
	TOctTree<GameObject, MyTreeOptions>::ElementIterator elementIterator(nodeContext.Node);
	while(elementIterator.MoveNext())
	{
		const GameObject& element = elementIterator.GetElement();
		RenderElement(element);
	}

	// Add intersecting child nodes for traversal
	auto childRange = nodeContext.Bounds.FindIntersectingChildren(simd::AABox(frustumBounds));
	for(u32 i = 0; i < 8; i++)
	{
		if(childRange.Contains(i) && nodeContext.Node->HasChild(i))
		{
			nodeIterator.PushChild(i);
		}
	}
}
~~~~~~~~~~~~~